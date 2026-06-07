package com.elleann.android.ui.shneditor

import android.net.Uri
import android.util.Base64 as AndroidBase64
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.elleann.android.data.ElleApiExtended
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.ByteArrayOutputStream
import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.Charset

data class SHNColumn(
    val name:   String,
    val type:   UInt,
    val length: Int,
)

data class SHNFile(
    val cryptHeader:         ByteArray      = ByteArray(0x20),
    val header:              UInt           = 0u,
    val columns:             List<SHNColumn> = emptyList(),
    val rows:                List<List<Any>> = emptyList(),
    val defaultRecordLength: UInt           = 0u,
)

enum class SHNEncoding(val displayName: String, val charsetName: String) {
    WIN1252   ("Windows-1252 (Western)", "windows-1252"),
    UTF8      ("UTF-8",                  "UTF-8"),
    EUC_KR    ("EUC-KR (Korean)",        "EUC-KR"),
    GB2312    ("GB2312 (Chinese)",       "GB2312"),
    ISO_8859_1("ISO-8859-1 (legacy)",    "ISO-8859-1");

    fun charset(): Charset = try { Charset.forName(charsetName) }
                             catch (_: Exception) { Charsets.ISO_8859_1 }
}

data class SHNState(
    val file:         SHNFile?    = null,
    val fileName:     String      = "",
    val serverRoot:   String      = "Hero",
    val loading:      Boolean     = false,
    val saving:       Boolean     = false,
    val error:        String?     = null,
    val dirtyRows:    Set<Int>    = emptySet(),
    val search:       String      = "",
    val filterCol:    Int         = 0,
    val encoding:     SHNEncoding = SHNEncoding.WIN1252,
    val statusBanner: String?     = null,
    val statusIsOk:   Boolean     = false,
    val serverFiles:  List<String> = emptyList(),
    val showBrowser:  Boolean     = false,
    val diffSummary:  DiffSummary? = null,

    val history:      List<com.elleann.android.data.models.ShnHistoryEntry> = emptyList(),
)

private fun shnCrypt(data: ByteArray) {
    if (data.isEmpty()) return
    var key = (data.size and 0xFF).toByte()
    for (i in data.indices.reversed()) {
        data[i] = (data[i].toInt() xor key.toInt()).toByte()
        var k = (i and 15).toByte()
        k = (k + 0x55).toByte()
        k = (k.toInt() xor ((i * 11) and 0xFF)).toByte()
        k = (k.toInt() xor key.toInt()).toByte()
        k = (k.toInt() xor 170).toByte()
        key = k
    }
}

private fun ByteBuffer.readFixedString(len: Int, charset: Charset): String {
    val bytes = ByteArray(len)
    get(bytes)
    val nullIdx = bytes.indexOf(0)
    val end = if (nullIdx >= 0) nullIdx else len
    return String(bytes, 0, end, charset)
}

private fun ByteBuffer.readNullTermString(charset: Charset): String {
    val buf = mutableListOf<Byte>()
    while (hasRemaining()) {
        val b = get()
        if (b == 0.toByte()) break
        buf.add(b)
    }
    return String(buf.toByteArray(), charset)
}

private fun parseSHN(raw: ByteArray, charset: Charset): SHNFile {
    if (raw.size < 0x28) throw IllegalArgumentException("File too small (<0x28 bytes)")

    val cryptHeader = raw.copyOf(0x20)
    val totalLen    = ByteBuffer.wrap(raw, 0x20, 4).order(ByteOrder.LITTLE_ENDIAN).int
    val payloadLen  = totalLen - 0x24
    if (payloadLen <= 0 || raw.size < 0x24 + payloadLen)
        throw IllegalArgumentException("Payload length mismatch: totalLen=$totalLen raw=${raw.size}")

    val payload = raw.copyOfRange(0x24, 0x24 + payloadLen)
    shnCrypt(payload)

    val bb = ByteBuffer.wrap(payload).order(ByteOrder.LITTLE_ENDIAN)

    val fileHeader    = bb.int.toUInt()
    val recordCount   = bb.int.toUInt()
    val defRecLen     = bb.int.toUInt()
    val columnCount   = bb.int
    if (columnCount <= 0 || columnCount > 4096)
        throw IllegalArgumentException("Unreasonable column count: $columnCount")

    var unkCols = 0
    val columns = (0 until columnCount).map {
        var name = bb.readFixedString(0x30, charset)
        if (name.isBlank()) { name = "UnkCol$unkCols"; unkCols++ }
        val type = bb.int.toUInt()
        val len  = bb.int
        SHNColumn(name, type, len)
    }

    val expectedRecLen = 2u + columns.sumOf { it.length.toUInt() }
    if (expectedRecLen != defRecLen) {
        throw IllegalArgumentException(
            "Wrong record length: header says $defRecLen, columns sum to $expectedRecLen. " +
            "Wrong encoding or corrupted file?")
    }

    val rows = (0u until recordCount).map {
        bb.short
        columns.map { col ->
            when (col.type.toInt()) {
                1, 12, 0x10       -> bb.get().toUByte().toLong()
                2                 -> bb.short.toUShort().toLong()
                3, 11, 0x12, 0x1b -> bb.int.toUInt().toLong()
                5                 -> bb.float.toDouble()
                13, 0x15          -> bb.short.toLong()
                0x16              -> bb.int.toLong()
                20                -> bb.get().toLong()
                9, 0x18           -> bb.readFixedString(col.length, charset)
                0x1a              -> bb.readNullTermString(charset)
                else              -> bb.get().toLong()
            }
        }
    }

    return SHNFile(
        cryptHeader         = cryptHeader,
        header              = fileHeader,
        columns             = columns,
        rows                = rows,
        defaultRecordLength = defRecLen,
    )
}

private fun serializeSHN(file: SHNFile, charset: Charset): ByteArray {
    val payloadOut = ByteArrayOutputStream()
    fun writeLE32(out: ByteArrayOutputStream, v: Int) {
        out.write(v and 0xFF); out.write((v shr 8) and 0xFF)
        out.write((v shr 16) and 0xFF); out.write((v shr 24) and 0xFF)
    }
    fun writeLE16(out: ByteArrayOutputStream, v: Int) {
        out.write(v and 0xFF); out.write((v shr 8) and 0xFF)
    }
    fun writeFloat(out: ByteArrayOutputStream, v: Float) {
        writeLE32(out, java.lang.Float.floatToIntBits(v))
    }
    fun writeFixedStr(out: ByteArrayOutputStream, s: String, len: Int) {
        val bytes = s.toByteArray(charset)
        val padded = ByteArray(len)
        bytes.copyInto(padded, 0, 0, minOf(len, bytes.size))
        out.write(padded)
    }
    fun writeNullStr(out: ByteArrayOutputStream, s: String) {
        out.write(s.toByteArray(charset)); out.write(0)
    }

    writeLE32(payloadOut, file.header.toInt())
    writeLE32(payloadOut, file.rows.size)

    val newDefRecLen = 2 + file.columns.sumOf { it.length }
    writeLE32(payloadOut, newDefRecLen)
    writeLE32(payloadOut, file.columns.size)

    for (col in file.columns) {
        val nameToWrite = if (col.name.startsWith("UnkCol")) "" else col.name
        writeFixedStr(payloadOut, nameToWrite, 0x30)
        writeLE32(payloadOut, col.type.toInt())
        writeLE32(payloadOut, col.length)
    }

    for (row in file.rows) {
        val rowBuf = ByteArrayOutputStream()
        writeLE16(rowBuf, 0)
        for ((col, value) in file.columns.zip(row)) {
            when (col.type.toInt()) {
                1, 12, 0x10       -> rowBuf.write((value as? Long ?: 0L).toInt() and 0xFF)
                2                 -> writeLE16(rowBuf, (value as? Long ?: 0L).toInt() and 0xFFFF)
                3, 11, 0x12, 0x1b -> writeLE32(rowBuf, (value as? Long ?: 0L).toInt())
                5                 -> writeFloat(rowBuf, (value as? Double ?: 0.0).toFloat())
                13, 0x15          -> writeLE16(rowBuf, (value as? Long ?: 0L).toInt() and 0xFFFF)
                0x16              -> writeLE32(rowBuf, (value as? Long ?: 0L).toInt())
                20                -> rowBuf.write((value as? Long ?: 0L).toInt() and 0xFF)
                9, 0x18           -> writeFixedStr(rowBuf, value as? String ?: "", col.length)
                0x1a              -> writeNullStr(rowBuf, value as? String ?: "")
                else              -> rowBuf.write((value as? Long ?: 0L).toInt() and 0xFF)
            }
        }
        val rowBytes = rowBuf.toByteArray()
        val rowLen = rowBytes.size
        rowBytes[0] = (rowLen and 0xFF).toByte()
        rowBytes[1] = ((rowLen shr 8) and 0xFF).toByte()
        payloadOut.write(rowBytes)
    }

    val payload = payloadOut.toByteArray()
    shnCrypt(payload)

    val result = ByteArrayOutputStream()
    result.write(file.cryptHeader)
    val totalLen = payload.size + 0x24
    writeLE32(result, totalLen)
    result.write(payload)
    return result.toByteArray()
}

private fun exportCsv(file: SHNFile, rows: List<List<Any>>): String {
    val sb = StringBuilder()
    file.columns.forEachIndexed { i, col ->
        sb.append(col.name)
        if (i + 1 < file.columns.size) sb.append(", ") else sb.append('\n')
    }
    for (row in rows) {
        row.forEachIndexed { i, v ->
            val s = v.toString().replace("\"", " ")
            sb.append('"').append(s).append('"')
            if (i + 1 < row.size) sb.append(',') else sb.append('\n')
        }
    }
    return sb.toString()
}

class SHNViewModel : ViewModel() {

    private val _state = MutableStateFlow(SHNState())
    val state: StateFlow<SHNState> = _state.asStateFlow()

    private val _editRows = mutableStateListOf<MutableList<Any>>()
    val editRows: List<MutableList<Any>> get() = _editRows

    private val _editColumns = mutableStateListOf<SHNColumn>()

    fun loadFromStream(stream: InputStream, name: String) {
        val enc = _state.value.encoding.charset()
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null, statusBanner = null) }
            runCatching {
                withContext(Dispatchers.IO) {
                    val bytes = stream.readBytes()
                    parseSHN(bytes, enc)
                }
            }.onSuccess { file ->
                _editRows.clear(); file.rows.forEach { _editRows.add(it.toMutableList()) }
                _editColumns.clear(); _editColumns.addAll(file.columns)
                val mismatch = detectEncodingMismatch(file)
                _state.update { it.copy(
                    loading = false, file = file, fileName = name,
                    dirtyRows = emptySet(), error = null,
                    statusBanner = if (mismatch != null)
                        "encoding mismatch suspected: $mismatch — switch encoding before editing"
                    else
                        "loaded ${file.rows.size} records · ${file.columns.size} columns",
                    statusIsOk = (mismatch == null)
                )}
            }.onFailure { e ->
                _state.update { it.copy(loading = false, error = e.message ?: "parse error") }
            }
        }
    }

    fun loadFromBytes(bytes: ByteArray, name: String) {
        val enc = _state.value.encoding.charset()
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null, statusBanner = null) }
            runCatching {
                withContext(Dispatchers.IO) { parseSHN(bytes, enc) }
            }.onSuccess { file ->
                _editRows.clear(); file.rows.forEach { _editRows.add(it.toMutableList()) }
                _editColumns.clear(); _editColumns.addAll(file.columns)

                val mismatch = detectEncodingMismatch(file)
                _state.update { it.copy(
                    loading = false, file = file, fileName = name,
                    dirtyRows = emptySet(), error = null,
                    statusBanner = if (mismatch != null)
                        "encoding mismatch suspected: $mismatch — try a different charset before editing"
                    else
                        "loaded from server: ${file.rows.size} records",
                    statusIsOk = (mismatch == null)
                )}
            }.onFailure { e ->
                _state.update { it.copy(loading = false, error = e.message ?: "parse error") }
            }
        }
    }

    private fun detectEncodingMismatch(file: SHNFile): String? {
        val strColIdx = file.columns.indexOfFirst {
            it.type.toInt() in listOf(9, 0x18, 0x1a)
        }
        if (strColIdx < 0) return null
        val sample = file.rows.take(8).mapNotNull {
            (it.getOrNull(strColIdx) as? String)?.takeIf(String::isNotBlank)
        }
        if (sample.isEmpty()) return null
        var bad = 0
        var total = 0
        for (s in sample) {
            for (c in s) {
                total++
                if (c == '\uFFFD' ||
                    (c.code < 0x20 && c != '\t' && c != '\n' && c != '\r')) bad++
            }
        }
        if (total == 0) return null
        val ratio = bad.toDouble() / total
        return if (ratio > 0.40)
            "${(ratio * 100).toInt()}% non-printable chars in '${file.columns[strColIdx].name}'"
        else null
    }

    fun setEncoding(e: SHNEncoding) = _state.update { it.copy(encoding = e) }
    fun setServerRoot(r: String) = _state.update { it.copy(serverRoot = r) }
    fun toggleBrowser() = _state.update { it.copy(showBrowser = !it.showBrowser) }
    fun setServerFiles(list: List<String>) =
        _state.update { it.copy(serverFiles = list, showBrowser = true) }

    fun setStatus(text: String?, ok: Boolean = false) =
        _state.update { it.copy(statusBanner = text, statusIsOk = ok) }

    fun computeAndShowDiff(serverBytes: ByteArray) {
        val enc = _state.value.encoding.charset()
        viewModelScope.launch {
            runCatching {
                withContext(Dispatchers.IO) { parseSHN(serverBytes, enc) }
            }.onSuccess { serverFile ->
                val localFile = snapshotFile()
                if (localFile == null) {
                    _state.update { it.copy(error = "no local file loaded") }
                    return@onSuccess
                }
                val summary = computeDiff(localFile, serverFile)
                _state.update { it.copy(
                    diffSummary = summary,
                    statusBanner = "diff: +${summary.added} / " +
                                   "−${summary.removed} / ~${summary.changed}",
                    statusIsOk   = summary.added + summary.removed + summary.changed == 0,
                )}
            }.onFailure { e ->
                _state.update { it.copy(
                    error = "diff parse failed: ${e.message ?: "unknown"}"
                )}
            }
        }
    }

    fun closeDiff() = _state.update { it.copy(diffSummary = null) }

    fun setHistory(list: List<com.elleann.android.data.models.ShnHistoryEntry>) =
        _state.update { it.copy(history = list) }

    fun setCell(rowIdx: Int, colIdx: Int, raw: String) {
        val col = _editColumns.getOrNull(colIdx) ?: return
        val parsed: Any = when (col.type.toInt()) {
            1, 12, 0x10, 2, 3, 11, 0x12, 0x1b, 13, 0x15, 0x16, 20 ->
                raw.toLongOrNull() ?: _editRows[rowIdx][colIdx]
            5    -> raw.toDoubleOrNull() ?: _editRows[rowIdx][colIdx]
            else -> raw
        }
        _editRows[rowIdx][colIdx] = parsed
        _state.update { it.copy(dirtyRows = it.dirtyRows + rowIdx) }
    }

    fun addRow() {
        if (_editColumns.isEmpty()) return
        val emptyRow: MutableList<Any> = _editColumns.map { col ->
            when (col.type.toInt()) {
                5    -> 0.0
                9, 0x18, 0x1a -> ""
                else -> 0L
            }
        }.toMutableList()
        _editRows.add(emptyRow)
        _state.update { it.copy(dirtyRows = it.dirtyRows + (_editRows.size - 1)) }
    }

    fun deleteRow(idx: Int) {
        if (idx !in _editRows.indices) return
        _editRows.removeAt(idx)
        _state.update { it.copy(dirtyRows = emptySet()) }
    }

    fun addColumn(name: String, type: UInt, length: Int) {
        if (name.isBlank()) return
        _editColumns.add(SHNColumn(name, type, length))

        val defaultVal: Any = when (type.toInt()) {
            5 -> 0.0; 9, 0x18, 0x1a -> ""; else -> 0L
        }
        for (r in _editRows) r.add(defaultVal)
        _state.update { it.copy(
            dirtyRows = _editRows.indices.toSet(),
            statusBanner = "added column '$name' (type=${type.toInt()}, len=$length)",
            statusIsOk = true
        )}
    }

    fun deleteColumn(idx: Int) {
        if (idx !in _editColumns.indices) return
        val removed = _editColumns.removeAt(idx)
        for (r in _editRows) if (idx < r.size) r.removeAt(idx)
        _state.update { it.copy(
            dirtyRows = _editRows.indices.toSet(),
            statusBanner = "deleted column '${removed.name}'",
            statusIsOk = true
        )}
    }

    fun renameColumn(idx: Int, newName: String) {
        if (idx !in _editColumns.indices || newName.isBlank()) return
        val old = _editColumns[idx]
        _editColumns[idx] = old.copy(name = newName.trim())
        _state.update { it.copy(
            dirtyRows = _editRows.indices.toSet(),
            statusBanner = "renamed '${old.name}' → '$newName'",
            statusIsOk = true
        )}
    }

    enum class BulkOp { Multiply, Divide, Add, Set }

    fun bulkOpColumn(colIdx: Int, op: BulkOp, factorStr: String): String? {
        if (colIdx !in _editColumns.indices) return "bad column"
        val col = _editColumns[colIdx]
        if (col.type.toInt() in listOf(9, 0x18, 0x1a))
            return "string columns can't be ${op.name.lowercase()}-ed; use Set on a row instead"
        val factor = factorStr.toDoubleOrNull() ?: return "factor must be a number"
        if (op == BulkOp.Divide && factor == 0.0) return "cannot divide by zero"

        val isFloat = col.type.toInt() == 5
        for (i in _editRows.indices) {
            val cur = _editRows[i].getOrNull(colIdx) ?: continue
            val curD = (cur as? Long)?.toDouble() ?: (cur as? Double) ?: 0.0
            val newD = when (op) {
                BulkOp.Multiply -> curD * factor
                BulkOp.Divide   -> curD / factor
                BulkOp.Add      -> curD + factor
                BulkOp.Set      -> factor
            }
            _editRows[i][colIdx] = if (isFloat) newD else newD.toLong()
        }
        _state.update { it.copy(
            dirtyRows = _editRows.indices.toSet(),
            statusBanner = "bulk ${op.name.lowercase()} on '${col.name}' by $factor",
            statusIsOk = true
        )}
        return null
    }

    fun sqlExport(): String? {
        val file = snapshotFile() ?: return null
        val tableName = state.value.fileName.removeSuffix(".shn").ifBlank { "shn_table" }
        val sb = StringBuilder()
        sb.append("-- Generated from ").append(state.value.fileName)
          .append(" (").append(file.rows.size).append(" rows)\n")
        sb.append("DROP TABLE IF EXISTS [").append(tableName).append("];\n")
        sb.append("CREATE TABLE [").append(tableName).append("] (\n")
        file.columns.forEachIndexed { i, col ->
            val sqlType = when (col.type.toInt()) {
                1, 12, 0x10  -> "TINYINT"
                2            -> "SMALLINT"
                3, 11, 0x12, 0x1b -> "BIGINT"
                5            -> "REAL"
                13, 0x15     -> "SMALLINT"
                0x16         -> "INT"
                20           -> "TINYINT"
                9, 0x18      -> "VARCHAR(${col.length})"
                0x1a         -> "VARCHAR(MAX)"
                else         -> "VARBINARY(${col.length})"
            }
            sb.append("  [").append(col.name).append("] ").append(sqlType)
            if (i + 1 < file.columns.size) sb.append(',')
            sb.append('\n')
        }
        sb.append(");\n\n")
        for (row in file.rows) {
            sb.append("INSERT INTO [").append(tableName).append("] VALUES (")
            row.forEachIndexed { i, v ->
                val col = file.columns[i]
                val isStr = col.type.toInt() in listOf(9, 0x18, 0x1a)
                if (isStr) {
                    sb.append('\'')
                      .append(v.toString().replace("'", "''"))
                      .append('\'')
                } else {
                    sb.append(v.toString())
                }
                if (i + 1 < row.size) sb.append(", ")
            }
            sb.append(");\n")
        }
        return sb.toString()
    }

    fun onSearch(q: String) = _state.update { it.copy(search = q) }
    fun onFilterCol(c: Int) = _state.update { it.copy(filterCol = c) }

    fun getFilteredIndices(): List<Int> {
        val s = _state.value
        val q = s.search.trim().lowercase()
        if (q.isEmpty()) return _editRows.indices.toList()
        return _editRows.indices.filter { i ->
            _editRows[i].getOrNull(s.filterCol)?.toString()?.lowercase()?.contains(q) == true
        }
    }

    fun currentColumns(): List<SHNColumn> = _editColumns.toList()

    fun snapshotFile(): SHNFile? {
        val base = _state.value.file ?: return null
        return base.copy(
            columns = _editColumns.toList(),
            rows    = _editRows.map { it.toList() }
        )
    }

    fun serialize(): ByteArray? {
        val file = snapshotFile() ?: return null
        return serializeSHN(file, _state.value.encoding.charset())
    }

    fun saveTo(onBytes: (ByteArray, String) -> Unit) {
        val file = snapshotFile() ?: return
        val name = _state.value.fileName
        val enc  = _state.value.encoding.charset()
        viewModelScope.launch {
            _state.update { it.copy(saving = true, error = null) }
            runCatching {
                withContext(Dispatchers.IO) { serializeSHN(file, enc) }
            }.onSuccess { bytes ->
                _state.update { it.copy(saving = false, dirtyRows = emptySet()) }
                onBytes(bytes, name)
            }.onFailure { e ->
                _state.update { it.copy(saving = false, error = e.message ?: "save error") }
            }
        }
    }

    fun csvExport(): String? {
        val file = snapshotFile() ?: return null
        return exportCsv(file, _editRows.map { it.toList() })
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SHNScreen(
    api:            ElleApiExtended? = null,
    onSaveToServer: ((root: String, name: String, bytes: ByteArray,
                      onResult: (ok: Boolean, msg: String) -> Unit) -> Unit)? = null,
) {
    val vm: SHNViewModel = viewModel()
    val state by vm.state.collectAsState()
    val ctx = LocalContext.current
    val scope = rememberCoroutineScope()

    val filePicker = rememberLauncherForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        if (uri == null) return@rememberLauncherForActivityResult
        val name = uri.lastPathSegment?.substringAfterLast('/') ?: "unknown.shn"

        val bytes: ByteArray? = runCatching {
            ctx.contentResolver.openInputStream(uri)?.use { it.readBytes() }
        }.getOrNull()
        if (bytes == null || bytes.isEmpty()) {
            Toast.makeText(ctx, "Could not read file (empty or denied)", Toast.LENGTH_LONG).show()
            vm.setStatus("could not read picked file (empty / denied)")
            return@rememberLauncherForActivityResult
        }
        vm.loadFromBytes(bytes, name)
    }

    val saveToDevice = rememberLauncherForActivityResult(
        ActivityResultContracts.CreateDocument("application/octet-stream")
    ) { uri: Uri? ->
        if (uri == null) return@rememberLauncherForActivityResult
        vm.saveTo { bytes, _ ->
            ctx.contentResolver.openOutputStream(uri)?.use { it.write(bytes) }
            vm.setStatus("saved to device (${bytes.size} bytes)", ok = true)
        }
    }

    val saveCsvToDevice = rememberLauncherForActivityResult(
        ActivityResultContracts.CreateDocument("text/csv")
    ) { uri: Uri? ->
        if (uri == null) return@rememberLauncherForActivityResult
        val csv = vm.csvExport() ?: run { vm.setStatus("no data to export"); return@rememberLauncherForActivityResult }
        ctx.contentResolver.openOutputStream(uri)?.use {
            it.write(csv.toByteArray(Charsets.UTF_8))
        }
        vm.setStatus("CSV exported (${csv.length} chars)", ok = true)
    }

    val saveSqlToDevice = rememberLauncherForActivityResult(
        ActivityResultContracts.CreateDocument("application/sql")
    ) { uri: Uri? ->
        if (uri == null) return@rememberLauncherForActivityResult
        val sql = vm.sqlExport() ?: run { vm.setStatus("no data to export"); return@rememberLauncherForActivityResult }
        ctx.contentResolver.openOutputStream(uri)?.use {
            it.write(sql.toByteArray(Charsets.UTF_8))
        }
        vm.setStatus("SQL exported (${sql.length} chars)", ok = true)
    }

    var addColDialog  by remember { mutableStateOf(false) }
    var bulkOpDialog  by remember { mutableStateOf<Int?>(null) }
    var renameColDialog by remember { mutableStateOf<Int?>(null) }
    var pendingSaveToServer by remember { mutableStateOf(false) }
    var pendingSaveToDevice by remember { mutableStateOf(false) }

    LaunchedEffect(state.fileName) {
        if (api != null && state.fileName.isNotEmpty()) {
            runCatching { api.historySHN(state.fileName, 5) }
                .onSuccess { vm.setHistory(it.entries) }
                .onFailure { vm.setHistory(emptyList()) }
        }
    }

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            IsyaTopBar(
                title    = { Text(if (state.fileName.isEmpty()) "SHN Editor" else state.fileName) },
                subtitle = state.file?.let { file ->
                    { Text("${file.rows.size} rec · ${vm.currentColumns().size} col · ${state.encoding.charsetName}") }
                },
                actions  = {
                    if (state.file != null) {

                        if (onSaveToServer != null) {
                            IconButton(onClick = {

                                pendingSaveToServer = true
                            }, enabled = !state.saving) {
                                if (state.saving)
                                    CircularProgressIndicator(Modifier.size(18.dp), color = IsyaMagic, strokeWidth = 2.dp)
                                else
                                    Icon(Icons.Rounded.CloudUpload, "Save to server", tint = IsyaMagic)
                            }
                        }

                        if (api != null) {
                            IconButton(onClick = {
                                scope.launch {
                                    runCatching {
                                        api.getSHN(state.serverRoot, state.fileName)
                                    }.onSuccess { resp ->
                                        val bytes = AndroidBase64.decode(
                                            resp.bytesB64, AndroidBase64.DEFAULT)
                                        vm.computeAndShowDiff(bytes)
                                    }.onFailure { e ->
                                        val msg = "diff fetch failed: ${e.message}"
                                        vm.setStatus(msg)
                                        Toast.makeText(ctx, msg, Toast.LENGTH_LONG).show()
                                    }
                                }
                            }) {
                                Icon(Icons.Rounded.CompareArrows, "Diff against server",
                                     tint = IsyaGold)
                            }
                        }
                        IconButton(onClick = { saveToDevice.launch(state.fileName) }) {
                            Icon(Icons.Rounded.SaveAlt, "Save to device", tint = IsyaGold)
                        }
                        IconButton(onClick = {
                            saveCsvToDevice.launch(state.fileName.removeSuffix(".shn") + ".csv")
                        }) {
                            Icon(Icons.Rounded.Description, "Export CSV", tint = IsyaCream)
                        }
                        IconButton(onClick = {
                            saveSqlToDevice.launch(state.fileName.removeSuffix(".shn") + ".sql")
                        }) {
                            Icon(Icons.Rounded.Storage, "Export SQL", tint = IsyaCream)
                        }
                        IconButton(onClick = { addColDialog = true }) {
                            Icon(Icons.Rounded.ViewColumn, "Add column", tint = IsyaSuccess)
                        }
                        IconButton(onClick = vm::addRow) {
                            Icon(Icons.Rounded.AddCircleOutline, "Add row", tint = IsyaSuccess)
                        }
                    }

                    if (api != null) {
                        IconButton(onClick = {
                            scope.launch {
                                runCatching { api.listSHN(state.serverRoot) }
                                    .onSuccess { vm.setServerFiles(it.files.map { e -> e.name }) }
                                    .onFailure { e ->
                                        vm.setStatus("list failed: ${e.message}")
                                        Toast.makeText(ctx, "list failed: ${e.message}",
                                            Toast.LENGTH_LONG).show()
                                    }
                            }
                        }) {
                            Icon(Icons.Rounded.CloudDownload, "Browse server", tint = IsyaMagic)
                        }
                    }

                    IconButton(onClick = { filePicker.launch("*/*") }) {
                        Icon(Icons.Rounded.FolderOpen, "Open SHN", tint = IsyaCream)
                    }
                }
            )
        }
    ) { padding ->
        Column(Modifier.fillMaxSize().padding(padding)) {

            Row(
                Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                Text("Root:", color = IsyaMuted, fontSize = 11.sp)
                listOf("Hero", "ReSystem").forEach { r ->
                    val active = state.serverRoot == r
                    FilterChip(
                        selected = active,
                        onClick  = { vm.setServerRoot(r) },
                        label    = { Text(r, fontSize = 10.sp) },
                    )
                }
                Spacer(Modifier.width(8.dp))
                Text("Enc:", color = IsyaMuted, fontSize = 11.sp)
                var encExpanded by remember { mutableStateOf(false) }
                Box {
                    OutlinedButton(
                        onClick = { encExpanded = true },
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 2.dp),
                    ) {
                        Text(state.encoding.charsetName, fontSize = 10.sp)
                        Icon(Icons.Rounded.ArrowDropDown, null, Modifier.size(14.dp))
                    }
                    DropdownMenu(
                        expanded = encExpanded,
                        onDismissRequest = { encExpanded = false },
                    ) {
                        SHNEncoding.values().forEach { e ->
                            DropdownMenuItem(
                                text = { Text(e.displayName, fontSize = 12.sp) },
                                onClick = { vm.setEncoding(e); encExpanded = false },
                            )
                        }
                    }
                }
            }

            state.error?.let { IsyaErrorState(it) }
            state.statusBanner?.let {
                Box(
                    Modifier.fillMaxWidth()
                        .background((if (state.statusIsOk) IsyaSuccess else IsyaWarn)
                            .copy(alpha = 0.15f))
                        .padding(8.dp),
                    contentAlignment = Alignment.Center,
                ) {
                    Text(it,
                        color    = if (state.statusIsOk) IsyaSuccess else IsyaWarn,
                        fontSize = 12.sp)
                }
            }

            if (state.history.isNotEmpty()) {
                val last = state.history.first()
                val ageMin = ((System.currentTimeMillis() - last.tsMs) / 60_000L).coerceAtLeast(0)
                val ageStr = when {
                    ageMin < 1     -> "just now"
                    ageMin < 60    -> "${ageMin}m ago"
                    ageMin < 1440  -> "${ageMin / 60}h ago"
                    else           -> "${ageMin / 1440}d ago"
                }
                Box(
                    Modifier.fillMaxWidth()
                        .background(IsyaMagic.copy(alpha = 0.08f))
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                ) {
                    Text(
                        "last saved $ageStr by ${last.user} · ${last.bytes}B " +
                        "· ${state.history.size} recent edits",
                        color    = IsyaMagic.copy(alpha = 0.85f),
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace,
                    )
                }
            }

            if (state.showBrowser && api != null) {
                ServerBrowser(
                    root  = state.serverRoot,
                    files = state.serverFiles,
                    onDismiss = { vm.toggleBrowser() },
                    onPick = { name ->
                        vm.toggleBrowser()
                        scope.launch {
                            runCatching { api.getSHN(state.serverRoot, name) }
                                .onSuccess { resp ->
                                    val bytes = AndroidBase64.decode(resp.bytesB64, AndroidBase64.DEFAULT)
                                    vm.loadFromBytes(bytes, name)
                                }
                                .onFailure { e ->
                                    vm.setStatus("fetch failed: ${e.message}")
                                    Toast.makeText(ctx, "fetch failed: ${e.message}",
                                        Toast.LENGTH_LONG).show()
                                }
                        }
                    }
                )
            }

            when {
                state.loading -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        CircularProgressIndicator(color = IsyaMagic)
                    }
                }

                state.file == null && !state.showBrowser -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally,
                               verticalArrangement = Arrangement.spacedBy(12.dp)) {
                            Icon(Icons.Rounded.TableChart, null, tint = IsyaMuted, modifier = Modifier.size(48.dp))
                            Text("Open a .shn file to edit", color = IsyaMuted, fontSize = 14.sp)
                            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                                OutlinedButton(onClick = { filePicker.launch("*/*") }) {
                                    Icon(Icons.Rounded.FolderOpen, null, Modifier.size(16.dp))
                                    Spacer(Modifier.width(4.dp))
                                    Text("Local file")
                                }
                                if (api != null) {
                                    OutlinedButton(onClick = {
                                        scope.launch {
                                            runCatching { api.listSHN(state.serverRoot) }
                                                .onSuccess { vm.setServerFiles(it.files.map { e -> e.name }) }
                                                .onFailure { e -> vm.setStatus("list failed: ${e.message}") }
                                        }
                                    }) {
                                        Icon(Icons.Rounded.CloudDownload, null, Modifier.size(16.dp))
                                        Spacer(Modifier.width(4.dp))
                                        Text("From server")
                                    }
                                }
                            }
                        }
                    }
                }

                state.file != null -> {
                    SHNTableView(
                        vm = vm, state = state, columns = vm.currentColumns(),
                        onBulkOp    = { idx -> bulkOpDialog = idx },
                        onRenameCol = { idx -> renameColDialog = idx },
                    )
                }
            }
        }
    }

    state.diffSummary?.let { summary ->
        SHNDiffView(
            summary   = summary,
            columns   = vm.currentColumns(),
            onDismiss = vm::closeDiff,
        )
    }

    if (addColDialog) {
        AddColumnDialog(
            onDismiss = { addColDialog = false },
            onAdd = { name, type, len ->
                vm.addColumn(name, type, len); addColDialog = false
            }
        )
    }

    bulkOpDialog?.let { colIdx ->
        BulkOpDialog(
            colName   = vm.currentColumns().getOrNull(colIdx)?.name ?: "",
            onDismiss = { bulkOpDialog = null },
            onApply   = { op, factor ->
                val err = vm.bulkOpColumn(colIdx, op, factor)
                if (err != null) {
                    Toast.makeText(ctx, err, Toast.LENGTH_LONG).show()
                } else {
                    bulkOpDialog = null
                }
            }
        )
    }

    renameColDialog?.let { colIdx ->
        val current = vm.currentColumns().getOrNull(colIdx)?.name ?: ""
        RenameColumnDialog(
            currentName = current,
            onDismiss   = { renameColDialog = null },
            onRename    = { newName ->
                vm.renameColumn(colIdx, newName); renameColDialog = null
            }
        )
    }

    if (pendingSaveToServer) {
        EncodingConfirmDialog(
            encodingDisplay = state.encoding.displayName,
            saveTarget      = "server (${state.serverRoot})",
            onDismiss       = { pendingSaveToServer = false },
            onConfirm       = {
                pendingSaveToServer = false
                vm.saveTo { bytes, name ->
                    onSaveToServer?.invoke(state.serverRoot, name, bytes) { ok, msg ->
                        vm.setStatus(msg, ok)
                        Toast.makeText(ctx, msg,
                            if (ok) Toast.LENGTH_SHORT else Toast.LENGTH_LONG
                        ).show()
                    }
                }
            }
        )
    }
}

@Composable
private fun EncodingConfirmDialog(
    encodingDisplay: String,
    saveTarget:      String,
    onDismiss:       () -> Unit,
    onConfirm:       () -> Unit,
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Confirm encoding before save", color = IsyaCream) },
        text  = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Active encoding: $encodingDisplay",
                     color = IsyaGold, fontFamily = FontFamily.Monospace, fontSize = 13.sp)
                Text("Saving to: $saveTarget",
                     color = IsyaCream, fontSize = 12.sp)
                Text(
                    "If this isn't the encoding the file was created with, " +
                    "every string column will be re-encoded into the wrong charset " +
                    "and the file will be permanently corrupted. " +
                    "If column[0] (usually the ID) looks like garbage when you " +
                    "view it, switch encoding (top bar) BEFORE saving.",
                    color = IsyaWarn, fontSize = 11.sp,
                )
            }
        },
        confirmButton = {
            TextButton(onClick = onConfirm) { Text("Save anyway") }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text("Cancel — let me check") }
        },
    )
}

@Composable
private fun BulkOpDialog(
    colName:   String,
    onDismiss: () -> Unit,
    onApply:   (SHNViewModel.BulkOp, String) -> Unit,
) {
    var op  by remember { mutableStateOf(SHNViewModel.BulkOp.Multiply) }
    var fct by remember { mutableStateOf("1") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Bulk op · '$colName'", color = IsyaCream) },
        text  = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Apply to every row's '$colName' cell:",
                     color = IsyaMuted, fontSize = 11.sp)
                Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
                    SHNViewModel.BulkOp.values().forEach { o ->
                        FilterChip(
                            selected = op == o,
                            onClick  = { op = o },
                            label    = { Text(o.name, fontSize = 11.sp) },
                        )
                    }
                }
                OutlinedTextField(
                    value = fct, onValueChange = { fct = it },
                    label = { Text(when (op) {
                        SHNViewModel.BulkOp.Multiply -> "Factor (× this)"
                        SHNViewModel.BulkOp.Divide   -> "Divisor (÷ this)"
                        SHNViewModel.BulkOp.Add      -> "Delta (+ this)"
                        SHNViewModel.BulkOp.Set      -> "Value (= this)"
                    }) },
                    singleLine = true,
                )
                Text("Skips string columns. Float columns keep precision; " +
                     "integer columns are truncated.",
                     color = IsyaMuted, fontSize = 10.sp)
            }
        },
        confirmButton = {
            TextButton(onClick = { onApply(op, fct) }) { Text("Apply") }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text("Cancel") }
        }
    )
}

@Composable
private fun RenameColumnDialog(
    currentName: String,
    onDismiss:   () -> Unit,
    onRename:    (String) -> Unit,
) {
    var name by remember(currentName) { mutableStateOf(currentName) }
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Rename column", color = IsyaCream) },
        text  = {
            OutlinedTextField(
                value = name, onValueChange = { name = it },
                label = { Text("New name") }, singleLine = true,
            )
        },
        confirmButton = {
            TextButton(onClick = {
                if (name.isNotBlank() && name != currentName) onRename(name.trim())
                else onDismiss()
            }) { Text("Rename") }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text("Cancel") }
        }
    )
}

@Composable
private fun ServerBrowser(
    root:      String,
    files:     List<String>,
    onDismiss: () -> Unit,
    onPick:    (String) -> Unit,
) {
    Column(
        Modifier.fillMaxWidth()
            .background(IsyaNight)
            .border(1.dp, IsyaMagic.copy(alpha = 0.3f))
            .padding(8.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text("Server · 9Data/$root", color = IsyaMagic, fontSize = 13.sp,
                 fontWeight = FontWeight.Bold, modifier = Modifier.weight(1f))
            IconButton(onClick = onDismiss, modifier = Modifier.size(24.dp)) {
                Icon(Icons.Rounded.Close, null, tint = IsyaMuted, modifier = Modifier.size(16.dp))
            }
        }
        if (files.isEmpty()) {
            Text("no .shn files found in 9Data/$root",
                 color = IsyaMuted, fontSize = 11.sp,
                 modifier = Modifier.padding(8.dp))
        } else {
            LazyColumn(Modifier.heightIn(max = 240.dp)) {
                items(files.size) { i ->
                    Text(files[i],
                        color = IsyaCream, fontSize = 12.sp,
                        fontFamily = FontFamily.Monospace,
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(vertical = 4.dp, horizontal = 8.dp)
                            .background(if (i % 2 == 0) IsyaNight else IsyaNight.copy(alpha = 0.7f))
                    )
                    TextButton(
                        onClick = { onPick(files[i]) },
                        modifier = Modifier.fillMaxWidth(),
                    ) { Text("Open", fontSize = 10.sp) }
                }
            }
        }
    }
}

@Composable
private fun AddColumnDialog(
    onDismiss: () -> Unit,
    onAdd:     (name: String, type: UInt, length: Int) -> Unit,
) {
    var name by remember { mutableStateOf("") }
    var typeStr by remember { mutableStateOf("3") }
    var lenStr  by remember { mutableStateOf("4") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Add column", color = IsyaCream) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(value = name, onValueChange = { name = it },
                    label = { Text("Name") }, singleLine = true)
                OutlinedTextField(value = typeStr, onValueChange = { typeStr = it.filter(Char::isDigit) },
                    label = { Text("Type (1=byte 2=u16 3=u32 5=float 13=i16 0x18=str)") },
                    singleLine = true)
                OutlinedTextField(value = lenStr, onValueChange = { lenStr = it.filter(Char::isDigit) },
                    label = { Text("Length (bytes)") }, singleLine = true)
                Text("Type 9/0x18 (fixed str) needs Length = buffer bytes. " +
                     "0x1a (null-term str) Length is unused at parse time but we " +
                     "store what you entered.",
                     color = IsyaMuted, fontSize = 10.sp)
            }
        },
        confirmButton = {
            TextButton(onClick = {
                val t = typeStr.toIntOrNull()?.toUInt() ?: 3u
                val l = lenStr.toIntOrNull() ?: 4
                if (name.isNotBlank() && l > 0) onAdd(name.trim(), t, l)
            }) { Text("Add") }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text("Cancel") }
        }
    )
}

@Composable
private fun SHNTableView(
    vm:      SHNViewModel,
    state:   SHNState,
    columns: List<SHNColumn>,
    onBulkOp:    (Int) -> Unit = {},
    onRenameCol: (Int) -> Unit = {},
) {
    val hScroll = rememberScrollState()

    Column(Modifier.fillMaxSize()) {
        Row(
            Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 4.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            IsyaSearchBar(
                value         = state.search,
                onValueChange = vm::onSearch,
                placeholder   = "Search column…",
                modifier      = Modifier.weight(1f),
            )
            if (columns.isNotEmpty()) {
                var expanded by remember { mutableStateOf(false) }
                Box {
                    OutlinedButton(
                        onClick = { expanded = true },
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp),
                    ) {
                        Text(
                            columns.getOrNull(state.filterCol)?.name?.take(10) ?: "Col",
                            fontSize = 11.sp,
                            maxLines = 1,
                        )
                        Icon(Icons.Rounded.ArrowDropDown, null, Modifier.size(16.dp))
                    }
                    DropdownMenu(
                        expanded = expanded,
                        onDismissRequest = { expanded = false },
                        modifier = Modifier.background(IsyaNight),
                    ) {
                        columns.forEachIndexed { i, col ->
                            DropdownMenuItem(
                                text = { Text(col.name, color = IsyaCream, fontSize = 12.sp,
                                              fontFamily = FontFamily.Monospace) },
                                onClick = { vm.onFilterCol(i); expanded = false },
                            )
                        }
                    }
                }
            }
        }

        Row(
            Modifier.fillMaxWidth().horizontalScroll(hScroll)
                .background(IsyaNight).border(1.dp, IsyaMuted.copy(alpha = 0.2f)),
        ) {
            SHNHeaderCell("#", 40.dp)
            columns.forEachIndexed { idx, col ->
                Row(verticalAlignment = Alignment.CenterVertically) {
                    SHNHeaderCell("${col.name}·T${col.type.toInt()}·L${col.length}", colWidth(col))
                    IconButton(onClick = { onRenameCol(idx) },
                               modifier = Modifier.size(24.dp)) {
                        Icon(Icons.Rounded.Edit, "Rename column",
                             tint = IsyaCream.copy(alpha = 0.5f),
                             modifier = Modifier.size(13.dp))
                    }
                    val isStr = col.type.toInt() in listOf(9, 0x18, 0x1a)
                    if (!isStr) {
                        IconButton(onClick = { onBulkOp(idx) },
                                   modifier = Modifier.size(24.dp)) {
                            Icon(Icons.Rounded.Calculate, "Bulk op",
                                 tint = IsyaGold.copy(alpha = 0.5f),
                                 modifier = Modifier.size(13.dp))
                        }
                    }
                    IconButton(onClick = { vm.deleteColumn(idx) },
                               modifier = Modifier.size(24.dp)) {
                        Icon(Icons.Rounded.RemoveCircleOutline, "Delete column",
                             tint = IsyaError.copy(alpha = 0.4f),
                             modifier = Modifier.size(13.dp))
                    }
                }
            }
            SHNHeaderCell("", 40.dp)
        }

        val indices = vm.getFilteredIndices()
        LazyColumn(Modifier.fillMaxSize()) {
            itemsIndexed(indices) { _, rowIdx ->
                val isDirty = rowIdx in state.dirtyRows
                Row(
                    Modifier.fillMaxWidth().horizontalScroll(hScroll)
                        .background(
                            if (isDirty) IsyaWarn.copy(alpha = 0.07f)
                            else if (rowIdx % 2 == 0) IsyaNight else IsyaNight.copy(alpha = 0.7f)
                        )
                        .border(
                            width = if (isDirty) 1.dp else 0.dp,
                            color = if (isDirty) IsyaWarn.copy(alpha = 0.3f) else IsyaNight,
                        ),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    Box(Modifier.width(40.dp).padding(2.dp),
                        contentAlignment = Alignment.Center) {
                        Text("$rowIdx", color = IsyaMuted, fontSize = 10.sp,
                             fontFamily = FontFamily.Monospace)
                    }
                    columns.forEachIndexed { colIdx, col ->
                        SHNCell(
                            value    = vm.editRows.getOrNull(rowIdx)?.getOrNull(colIdx)?.toString() ?: "",
                            width    = colWidth(col),
                            isString = col.type.toInt() in listOf(9, 0x18, 0x1a),
                            onChange = { vm.setCell(rowIdx, colIdx, it) },
                        )
                    }
                    IconButton(
                        onClick  = { vm.deleteRow(rowIdx) },
                        modifier = Modifier.size(40.dp),
                    ) {
                        Icon(Icons.Rounded.DeleteOutline, "Delete row",
                             tint = IsyaError.copy(alpha = 0.6f),
                             modifier = Modifier.size(16.dp))
                    }
                }
            }
            item { Spacer(Modifier.height(80.dp)) }
        }
    }
}

@Composable
private fun SHNHeaderCell(label: String, width: androidx.compose.ui.unit.Dp) {
    Box(
        Modifier.width(width).padding(horizontal = 4.dp, vertical = 6.dp),
        contentAlignment = Alignment.CenterStart,
    ) {
        Text(
            label, color = IsyaGold, fontSize = 10.sp,
            fontWeight = FontWeight.Bold, fontFamily = FontFamily.Monospace,
            maxLines = 1, overflow = TextOverflow.Ellipsis,
        )
    }
}

@Composable
private fun SHNCell(
    value:    String,
    width:    androidx.compose.ui.unit.Dp,
    isString: Boolean,
    onChange: (String) -> Unit,
) {
    var text by remember(value) { mutableStateOf(value) }

    BasicTextField(
        value = text,
        onValueChange = { text = it; onChange(it) },
        singleLine    = !isString,
        textStyle     = LocalTextStyle.current.copy(
            color      = if (isString) IsyaMagic.copy(alpha = 0.9f) else IsyaCream,
            fontSize   = 11.sp,
            fontFamily = FontFamily.Monospace,
        ),
        cursorBrush   = SolidColor(IsyaMagic),
        modifier      = Modifier.width(width).padding(horizontal = 4.dp, vertical = 4.dp),
    )
}

private fun colWidth(col: SHNColumn): androidx.compose.ui.unit.Dp = when (col.type.toInt()) {
    9, 0x18, 0x1a -> maxOf(120, col.length * 7).coerceAtMost(240).dp
    5             -> 80.dp
    3, 11, 0x12   -> 80.dp
    else          -> 64.dp
}
