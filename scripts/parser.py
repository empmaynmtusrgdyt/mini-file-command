import json
import sqlite3

conn = sqlite3.connect('data/magic.db')
cur = conn.cursor()

cur.execute("DROP TABLE IF EXISTS signatures")

cur.execute(
    '''CREATE TABLE signatures(
        id INTEGER PRIMARY KEY,
        extension TEXT,
        description TEXT,
        hex_sig BLOB, 
        sig_length INTEGER,
        offset INTEGER DEFAULT 0,
        trailer BLOB
    )'''
)

cur.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_sig_full ON signatures (hex_sig, sig_length, offset, trailer, description, extension)")

def hex_to_bytes(hex_string):
    if not hex_string: return None
    if hex_string == "(null)": return None
    hex_without_spaces = hex_string.replace(" ", "")
    if not all(c in "0123456789ABCDEFabcdef" for c in hex_without_spaces): return None
    if len(hex_without_spaces) % 2 != 0: return None
    return bytes.fromhex(hex_without_spaces)

def parse_offset(offset_str):
    if not offset_str:
        return 0
    try:
        return int(offset_str)
    except ValueError:
        i = 0
        while i < len(offset_str) and not offset_str[i].isdigit():
            i += 1
        if i == len(offset_str):
            return 0
        j = i
        while j < len(offset_str) and offset_str[j].isdigit():
            j += 1
        return int(offset_str[i:j])

with open('data/file_sigs.json') as f:
    data = json.load(f);

for entry in data.get("filesigs", []):
    header_hex = entry.get("Header (hex)")
    if not header_hex: continue

    binary_sig = hex_to_bytes(header_hex)
    if binary_sig is None: continue

    string_of_extensions = entry.get("File extension", "")
    extensions = [e.strip() for e in string_of_extensions.split("|") if e.strip()]
    if not extensions: extensions = ["*"]
    
    description = entry.get("File description", "Unknown")
    offset = parse_offset(entry.get("Header offset", "0"))
    trailer_hex = entry.get("Trailer (hex)", "(null)")
    trailer_bytes = hex_to_bytes(trailer_hex)

    sig_length = len(binary_sig)

    for ext in extensions:
        cur.execute("INSERT OR IGNORE INTO signatures (extension, description, hex_sig, sig_length, offset, trailer) VALUES (?, ?, ?, ?, ?, ?)",
                    (ext, description, binary_sig, sig_length, offset, trailer_bytes))

conn.commit()
conn.close()