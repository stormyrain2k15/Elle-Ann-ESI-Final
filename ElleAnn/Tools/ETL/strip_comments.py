import argparse
import io
import re
import sys
import tokenize
from pathlib import Path

ROOT = Path("/app/ElleAnn")

SKIP_DIRS = {
    "build", "_deps", "Testing", "output", "__pycache__", "node_modules",
    ".git", ".emergent", "_re_artifacts", "_winstub",
}
SKIP_PATH_PARTS = (
    "Tools/etl/output",
    "Tools/etl/data",
    "build/",
    "/_deps/",
    "_re_artifacts/",
)

C_LIKE_EXT = {".c", ".cc", ".cpp", ".cxx", ".c++", ".h", ".hh", ".hpp",
              ".hxx", ".h++", ".cs", ".kt", ".kts", ".java", ".m", ".mm",
              ".inc"}
PY_EXT = {".py"}
SQL_EXT = {".sql"}
LUA_EXT = {".lua"}
ASM_EXT = {".asm"}
SH_EXT = {".sh"}

ALL_EXT = C_LIKE_EXT | PY_EXT | SQL_EXT | LUA_EXT | ASM_EXT | SH_EXT

def should_skip(path: Path) -> bool:
    parts = path.parts
    for d in SKIP_DIRS:
        if d in parts:
            return True
    s = str(path)
    for p in SKIP_PATH_PARTS:
        if p in s:
            return True
    return False

def collapse_blank_lines(text: str) -> str:
    text = re.sub(r"\n[ \t]+\n", "\n\n", text)
    text = re.sub(r"\n{3,}", "\n\n", text)
    return text.lstrip("\n").rstrip() + "\n"

def strip_c_like(src: str) -> str:
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        nxt = src[i + 1] if i + 1 < n else ""
        if c == '"':
            j = i + 1
            if i >= 1 and src[i - 1] == "R" and j < n and src[j] == "(":
                end = src.find(')"', j + 1)
                if end == -1:
                    out.append(src[i:])
                    break
                out.append(src[i:end + 2])
                i = end + 2
                continue
            out.append(c)
            i += 1
            while i < n:
                ch = src[i]
                out.append(ch)
                i += 1
                if ch == "\\" and i < n:
                    out.append(src[i])
                    i += 1
                    continue
                if ch == '"':
                    break
            continue
        if c == "'":
            out.append(c)
            i += 1
            while i < n:
                ch = src[i]
                out.append(ch)
                i += 1
                if ch == "\\" and i < n:
                    out.append(src[i])
                    i += 1
                    continue
                if ch == "'":
                    break
            continue
        if c == "/" and nxt == "/":
            j = src.find("\n", i)
            if j == -1:
                break
            i = j
            continue
        if c == "/" and nxt == "*":
            j = src.find("*/", i + 2)
            if j == -1:
                break
            i = j + 2
            continue
        out.append(c)
        i += 1
    return collapse_blank_lines("".join(out))

def strip_python(src: str) -> str:
    try:
        tokens = list(tokenize.tokenize(io.BytesIO(src.encode("utf-8")).readline))
    except (tokenize.TokenizeError, IndentationError, SyntaxError):
        return src
    kept = [t for t in tokens if t.type != tokenize.COMMENT]
    docstring_positions = set()
    line_starts_with_string = {}
    expecting_docstring_at_indent = {0: True}
    prev_significant = None
    for idx, t in enumerate(kept):
        if t.type in (tokenize.NL, tokenize.NEWLINE, tokenize.ENCODING,
                      tokenize.ENDMARKER, tokenize.COMMENT):
            continue
        if t.type == tokenize.INDENT:
            expecting_docstring_at_indent[len(t.string)] = True
            continue
        if t.type == tokenize.DEDENT:
            continue
        if t.type == tokenize.STRING:
            col = t.start[1]
            if expecting_docstring_at_indent.get(col, False):
                if t.string.startswith(('"""', "'''", 'r"""', "r'''",
                                        'b"""', "b'''", 'R"""', "R'''",
                                        'rb"""', "rb'''", 'Rb"""', "Rb'''",
                                        'rB"""', "rB'''", 'br"""', "br'''")):
                    docstring_positions.add(idx)
        if prev_significant is None or (
            prev_significant.type == tokenize.OP and prev_significant.string == ":"
            and prev_significant.start[0] != t.start[0]
        ):
            pass
        if t.type == tokenize.NAME and t.string in ("def", "class"):
            expecting_docstring_at_indent.clear()
        else:
            for k in list(expecting_docstring_at_indent.keys()):
                if expecting_docstring_at_indent.get(k) and t.type not in (
                    tokenize.INDENT, tokenize.DEDENT,
                    tokenize.NEWLINE, tokenize.NL):
                    expecting_docstring_at_indent[k] = False
        prev_significant = t

    final = []
    for idx, t in enumerate(kept):
        if idx in docstring_positions:
            r0, c0 = t.start
            r1, c1 = t.end
            spaces = " " * c0
            final.append(tokenize.TokenInfo(
                tokenize.STRING, '""', t.start, t.end, t.line))
            continue
        final.append(t)
    try:
        out = tokenize.untokenize(final)
        if isinstance(out, bytes):
            out = out.decode("utf-8")
    except ValueError:
        return src

    out = re.sub(r'^\s*""\s*$\n?', "", out, flags=re.MULTILINE)
    return collapse_blank_lines(out)

def strip_sql(src: str) -> str:
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        nxt = src[i + 1] if i + 1 < n else ""
        if c == "'":
            out.append(c)
            i += 1
            while i < n:
                ch = src[i]
                out.append(ch)
                i += 1
                if ch == "'":
                    if i < n and src[i] == "'":
                        out.append("'")
                        i += 1
                        continue
                    break
            continue
        if c == "-" and nxt == "-":
            j = src.find("\n", i)
            if j == -1:
                break
            i = j
            continue
        if c == "/" and nxt == "*":
            j = src.find("*/", i + 2)
            if j == -1:
                break
            i = j + 2
            continue
        out.append(c)
        i += 1
    return collapse_blank_lines("".join(out))

def strip_lua(src: str) -> str:
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        nxt = src[i + 1] if i + 1 < n else ""
        if c == '"' or c == "'":
            quote = c
            out.append(c)
            i += 1
            while i < n:
                ch = src[i]
                out.append(ch)
                i += 1
                if ch == "\\" and i < n:
                    out.append(src[i]); i += 1; continue
                if ch == quote:
                    break
            continue
        if c == "[":
            m = re.match(r"\[(=*)\[", src[i:])
            if m:
                eqs = m.group(1)
                closer = "]" + eqs + "]"
                end = src.find(closer, i + len(m.group(0)))
                if end == -1:
                    out.append(src[i:]); break
                out.append(src[i:end + len(closer)])
                i = end + len(closer)
                continue
        if c == "-" and nxt == "-":
            m = re.match(r"--\[(=*)\[", src[i:])
            if m:
                eqs = m.group(1)
                closer = "]" + eqs + "]"
                end = src.find(closer, i + len(m.group(0)))
                if end == -1:
                    break
                i = end + len(closer)
                continue
            j = src.find("\n", i)
            if j == -1:
                break
            i = j
            continue
        out.append(c)
        i += 1
    return collapse_blank_lines("".join(out))

def strip_asm(src: str) -> str:
    out_lines = []
    for line in src.splitlines():
        in_str = False
        quote = ""
        cut = -1
        for k, ch in enumerate(line):
            if in_str:
                if ch == quote:
                    in_str = False
            else:
                if ch in ('"', "'"):
                    in_str = True; quote = ch
                elif ch == ";":
                    cut = k; break
        if cut >= 0:
            line = line[:cut].rstrip()
        if line.strip():
            out_lines.append(line)
        else:
            out_lines.append("")
    return collapse_blank_lines("\n".join(out_lines))

def strip_shell(src: str) -> str:
    out_lines = []
    first = True
    for line in src.splitlines():
        if first and line.startswith("#!"):
            out_lines.append(line); first = False; continue
        first = False
        in_str = False
        quote = ""
        cut = -1
        for k, ch in enumerate(line):
            if in_str:
                if ch == "\\":
                    pass
                elif ch == quote:
                    in_str = False
            else:
                if ch in ('"', "'"):
                    in_str = True; quote = ch
                elif ch == "#":
                    if k == 0 or line[k - 1].isspace():
                        cut = k; break
        if cut >= 0:
            line = line[:cut].rstrip()
        out_lines.append(line)
    return collapse_blank_lines("\n".join(out_lines))

def strip_for_ext(ext: str, src: str) -> str:
    if ext in C_LIKE_EXT:
        return strip_c_like(src)
    if ext in PY_EXT:
        return strip_python(src)
    if ext in SQL_EXT:
        return strip_sql(src)
    if ext in LUA_EXT:
        return strip_lua(src)
    if ext in ASM_EXT:
        return strip_asm(src)
    if ext in SH_EXT:
        return strip_shell(src)
    return src

def walk(roots):
    for root in roots:
        root = Path(root)
        if root.is_file():
            yield root; continue
        for p in root.rglob("*"):
            if p.is_file() and p.suffix.lower() in ALL_EXT and not should_skip(p):
                yield p

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--paths", nargs="*", default=[str(ROOT)])
    args = ap.parse_args()

    total = changed = bytes_saved = 0
    for path in walk(args.paths):
        try:
            src = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        total += 1
        new = strip_for_ext(path.suffix.lower(), src)
        if new != src:
            changed += 1
            saved = len(src) - len(new)
            bytes_saved += saved
            if not args.dry_run:
                path.write_text(new, encoding="utf-8")
    print(f"[strip] scanned {total} files; changed {changed}; "
          f"removed {bytes_saved:,} bytes "
          f"({'DRY RUN' if args.dry_run else 'WRITTEN'})")

if __name__ == "__main__":
    main()
