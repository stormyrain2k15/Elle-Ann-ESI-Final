package com.elleann.android.ui.shneditor

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Close
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.elleann.android.ui.theme.*

enum class DiffKind { Added, Removed, Changed }

data class DiffRow(
    val kind:         DiffKind,
    val key:          String,
    val localCells:   List<String>,
    val serverCells:  List<String>,

    val changedCols:  Set<Int>,
)

data class DiffSummary(
    val added:   Int,
    val removed: Int,
    val changed: Int,
    val rows:    List<DiffRow>,

    val columnSchemaDelta: String?,
)

private fun pickKey(row: List<Any>, cols: List<SHNColumn>): String {
    if (cols.isEmpty() || row.isEmpty()) return ""

    val c0 = cols[0]
    val isIntKey = c0.type.toInt() in
        listOf(1, 2, 3, 11, 12, 13, 20, 0x10, 0x12, 0x15, 0x16, 0x1b)
    return if (isIntKey) row[0].toString()
           else          row.joinToString("|") { it.toString() }
}

fun computeDiff(local: SHNFile, server: SHNFile): DiffSummary {
    val localByKey = LinkedHashMap<String, List<Any>>()
    for (r in local.rows) localByKey[pickKey(r, local.columns)] = r

    val serverByKey = LinkedHashMap<String, List<Any>>()
    for (r in server.rows) serverByKey[pickKey(r, server.columns)] = r

    val out = mutableListOf<DiffRow>()
    var added = 0; var removed = 0; var changed = 0

    val schemaDelta: String? = run {
        val ln = local.columns.map  { "${it.name}:${it.type.toInt()}" }
        val sn = server.columns.map { "${it.name}:${it.type.toInt()}" }
        if (ln == sn) null
        else "schema differs: local=${local.columns.size} cols, " +
             "server=${server.columns.size} cols " +
             "(first diff at col ${ln.zip(sn).indexOfFirst { it.first != it.second }})"
    }

    for ((k, lrow) in localByKey) {
        val srow = serverByKey[k]
        if (srow == null) {
            added++
            out.add(DiffRow(
                kind = DiffKind.Added, key = k,
                localCells  = lrow.map { it.toString() },
                serverCells = emptyList(),
                changedCols = emptySet(),
            ))
        } else {
            val diffs = mutableSetOf<Int>()
            val n = minOf(lrow.size, srow.size)
            for (i in 0 until n) {
                if (lrow[i].toString() != srow[i].toString()) diffs.add(i)
            }

            if (lrow.size != srow.size) {
                for (i in n until maxOf(lrow.size, srow.size)) diffs.add(i)
            }
            if (diffs.isNotEmpty()) {
                changed++
                out.add(DiffRow(
                    kind = DiffKind.Changed, key = k,
                    localCells  = lrow.map { it.toString() },
                    serverCells = srow.map { it.toString() },
                    changedCols = diffs,
                ))
            }
        }
    }

    for ((k, srow) in serverByKey) {
        if (!localByKey.containsKey(k)) {
            removed++
            out.add(DiffRow(
                kind = DiffKind.Removed, key = k,
                localCells  = emptyList(),
                serverCells = srow.map { it.toString() },
                changedCols = emptySet(),
            ))
        }
    }

    return DiffSummary(
        added = added, removed = removed, changed = changed,
        rows = out, columnSchemaDelta = schemaDelta,
    )
}

@androidx.compose.runtime.Composable
fun SHNDiffView(
    summary:   DiffSummary,
    columns:   List<SHNColumn>,
    onDismiss: () -> Unit,
) {
    val hScroll = rememberScrollState()

    Column(
        Modifier.fillMaxSize().background(IsyaNight)
            .border(1.dp, IsyaMagic.copy(alpha = 0.4f))
    ) {
        Row(
            Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Text("Diff preview · " +
                 "+${summary.added} / −${summary.removed} / ~${summary.changed}",
                 color = IsyaGold, fontSize = 13.sp, fontWeight = FontWeight.Bold,
                 modifier = Modifier.weight(1f))
            IconButton(onClick = onDismiss) {
                Icon(Icons.Rounded.Close, null, tint = IsyaMuted,
                     modifier = Modifier.size(18.dp))
            }
        }

        summary.columnSchemaDelta?.let {
            Box(
                Modifier.fillMaxWidth()
                    .background(IsyaWarn.copy(alpha = 0.15f))
                    .padding(8.dp)
            ) {
                Text(it, color = IsyaWarn, fontSize = 11.sp,
                     fontFamily = FontFamily.Monospace)
            }
        }

        if (summary.rows.isEmpty()) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                Text("no row-level changes — server and local are identical",
                     color = IsyaSuccess, fontSize = 12.sp)
            }
            return@Column
        }

        Row(
            Modifier.fillMaxWidth().horizontalScroll(hScroll)
                .border(1.dp, IsyaMuted.copy(alpha = 0.2f))
                .padding(vertical = 4.dp),
        ) {
            DiffHeaderCell("±",   32.dp)
            DiffHeaderCell("key", 80.dp)
            columns.forEach { col ->
                DiffHeaderCell("${col.name}", 100.dp)
            }
        }

        LazyColumn(Modifier.fillMaxSize()) {
            items(summary.rows) { dr ->
                val bg = when (dr.kind) {
                    DiffKind.Added   -> IsyaSuccess.copy(alpha = 0.10f)
                    DiffKind.Removed -> IsyaError.copy(alpha = 0.10f)
                    DiffKind.Changed -> IsyaWarn.copy(alpha = 0.08f)
                }
                val marker = when (dr.kind) {
                    DiffKind.Added   -> "+"
                    DiffKind.Removed -> "−"
                    DiffKind.Changed -> "~"
                }
                Row(
                    Modifier.fillMaxWidth().horizontalScroll(hScroll)
                        .background(bg)
                        .border(
                            width = 0.5.dp,
                            color = IsyaMuted.copy(alpha = 0.15f),
                        )
                        .padding(vertical = 2.dp),
                ) {
                    DiffCell(marker, 32.dp, when (dr.kind) {
                        DiffKind.Added   -> IsyaSuccess
                        DiffKind.Removed -> IsyaError
                        DiffKind.Changed -> IsyaWarn
                    })
                    DiffCell(dr.key, 80.dp, IsyaGold)

                    when (dr.kind) {
                        DiffKind.Added -> {
                            dr.localCells.forEach { cell ->
                                DiffCell(cell, 100.dp, IsyaSuccess)
                            }
                        }
                        DiffKind.Removed -> {
                            dr.serverCells.forEach { cell ->
                                DiffCell(cell, 100.dp, IsyaError.copy(alpha = 0.8f))
                            }
                        }
                        DiffKind.Changed -> {
                            dr.localCells.forEachIndexed { i, cell ->
                                val diff = i in dr.changedCols
                                DiffCell(
                                    text  = if (diff) cell + " ← " +
                                              (dr.serverCells.getOrNull(i) ?: "?")
                                            else cell,
                                    width = if (diff) 180.dp else 100.dp,
                                    color = if (diff) IsyaWarn else IsyaCream,
                                )
                            }
                        }
                    }
                }
            }
        }
    }
}

@androidx.compose.runtime.Composable
private fun DiffHeaderCell(label: String, width: androidx.compose.ui.unit.Dp) {
    Box(
        Modifier.width(width).padding(horizontal = 4.dp),
        contentAlignment = Alignment.CenterStart,
    ) {
        Text(label, color = IsyaMagic, fontSize = 10.sp,
             fontWeight = FontWeight.Bold, fontFamily = FontFamily.Monospace)
    }
}

@androidx.compose.runtime.Composable
private fun DiffCell(
    text:  String,
    width: androidx.compose.ui.unit.Dp,
    color: Color,
) {
    Box(
        Modifier.width(width).padding(horizontal = 4.dp, vertical = 2.dp),
        contentAlignment = Alignment.CenterStart,
    ) {
        Text(text, color = color, fontSize = 10.sp,
             fontFamily = FontFamily.Monospace,
             maxLines = 1,
             overflow = androidx.compose.ui.text.style.TextOverflow.Ellipsis)
    }
}
