import json
import sqlite3

conn = sqlite3.connect('magic.db')
cur = conn.cursor()

cur.execute(
    '''CREATE TABLE IF NOT EXISTS signatures(
        id INTEGER PRIMARY KEY,
        extension TEXT,
        description TEXT,
        hex_sig BLOB
    )'''
)

cur.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_sig_desc ON signatures (hex_sig, description)")

with open('file-signatures.json') as f:
    data = json.load(f);

for hex_string, info_list in data.items():
    try:
        hex_without_spaces = hex_string.replace(" ", "")
        binary_sig = bytes.fromhex(hex_without_spaces)
    except ValueError:
        print(f"Ошибка в формате hex: {hex_string}")
        continue

    for item in info_list:
        ext = item.get("extension", "*")
        des = item.get("description", "Unknown")

        cur.execute(
            "INSERT OR IGNORE INTO signatures (extension, description, hex_sig) VALUES (?, ?, ?)",
            (ext, des, binary_sig)
        ) 

conn.commit()
conn.close()