import struct, os, csv, sys

def decrypt_shn(data):
    buf = bytearray(data)
    length = len(buf)
    num = length & 0xFF
    for i in range(length - 1, -1, -1):
        buf[i] ^= num
        num3 = i & 15
        num3 = (num3 + 0x55) & 0xFF
        num3 ^= ((i * 11) & 0xFF)
        num3 ^= num
        num3 ^= 170
        num = num3
    return bytes(buf)

def parse_shn(path):
    with open(path, 'rb') as f:
        raw = f.read()

    crypt_header = raw[:0x20]

    file_size = struct.unpack_from('<I', raw, 0x20)[0]

    body_len = file_size - 0x24
    if body_len <= 0 or body_len > len(raw) - 0x24:
        body_len = len(raw) - 0x24
    encrypted = raw[0x24 : 0x24 + body_len]

    dec = decrypt_shn(encrypted)
    pos = 0

    def read_uint32():
        nonlocal pos
        v = struct.unpack_from('<I', dec, pos)[0]; pos += 4; return v
    def read_int32():
        nonlocal pos
        v = struct.unpack_from('<i', dec, pos)[0]; pos += 4; return v
    def read_uint16():
        nonlocal pos
        v = struct.unpack_from('<H', dec, pos)[0]; pos += 2; return v
    def read_int16():
        nonlocal pos
        v = struct.unpack_from('<h', dec, pos)[0]; pos += 2; return v
    def read_uint8():
        nonlocal pos
        v = dec[pos]; pos += 1; return v
    def read_int8():
        nonlocal pos
        v = struct.unpack_from('<b', dec, pos)[0]; pos += 1; return v
    def read_float():
        nonlocal pos
        v = struct.unpack_from('<f', dec, pos)[0]; pos += 4; return round(v, 6)
    def read_str_fixed(length):
        nonlocal pos
        raw_s = dec[pos:pos+length]; pos += length
        return raw_s.split(b'\x00')[0].decode('latin-1', errors='replace').strip()
    def read_str_null():
        nonlocal pos
        end = dec.index(b'\x00', pos)
        s = dec[pos:end].decode('latin-1', errors='replace')
        pos = end + 1
        return s

    header       = read_uint32()
    record_count = read_uint32()
    record_len   = read_uint32()
    col_count    = read_uint32()

    cols = []
    rec_size_check = 2
    for _ in range(col_count):
        name  = read_str_fixed(0x30)
        ctype = read_uint32()
        clen  = read_int32()
        cols.append({'name': name, 'type': ctype, 'len': clen})
        rec_size_check += clen

    rows = []
    for _ in range(record_count):
        read_uint16()
        row = {}
        for col in cols:
            t = col['type']
            l = col['len']
            if   t == 1:    v = read_uint8()
            elif t == 2:    v = read_uint16()
            elif t == 3:    v = read_uint32()
            elif t == 5:    v = read_float()
            elif t == 9:    v = read_str_fixed(l)
            elif t == 11:   v = read_uint32()
            elif t == 12:   v = read_uint8()
            elif t == 13:   v = read_int16()
            elif t == 0x10: v = read_uint8()
            elif t == 0x12: v = read_uint32()
            elif t == 20:   v = read_int8()
            elif t == 0x15: v = read_int16()
            elif t == 0x16: v = read_int32()
            elif t == 0x18: v = read_str_fixed(l)
            elif t == 0x1a: v = read_str_null()
            elif t == 0x1b: v = read_uint32()
            else:

                pos += l; v = f'?t{t}'
            row[col['name']] = v
        rows.append(row)

    return cols, rows

def shn_to_csv(shn_path, csv_path):
    cols, rows = parse_shn(shn_path)
    if not rows:
        print(f"  No rows in {shn_path}")
        return
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        w = csv.DictWriter(f, fieldnames=[c['name'] for c in cols])
        w.writeheader()
        w.writerows(rows)
    print(f"  -> {csv_path} ({len(rows)} rows, {len(cols)} cols)")

if __name__ == '__main__':
    import glob
    SHINE = '/home/claude/src/na2016/NA2016-main/Server/9Data/Shine'
    OUT   = '/home/claude/shn_csv'
    os.makedirs(OUT, exist_ok=True)

    targets = [
        'MoverMain.shn', 'MoverAbility.shn',
        'DamageLvGapPVE.shn', 'DamageLvGapPVP.shn', 'DamageLvGapEVP.shn',
        'ReactionType.shn',
        'MobInfoServer.shn', 'MobWeapon.shn', 'MobResist.shn',
        'ActiveSkillInfoServer.shn', 'ActiveSkill.shn',
        'PassiveSkill.shn', 'ItemInfo.shn', 'WeaponAttrib.shn',
        'AbState.shn', 'StateField.shn', 'StateMob.shn',
    ]
    for fname in targets:
        fpath = os.path.join(SHINE, fname)
        if not os.path.exists(fpath):
            print(f"  MISSING: {fname}")
            continue
        try:
            shn_to_csv(fpath, os.path.join(OUT, fname.replace('.shn','.csv')))
        except Exception as e:
            print(f"  ERROR {fname}: {e}")
