#!/usr/bin/env python3
"""
Create a test .mo file for i18n testing.
This generates a simple German translation file.
"""

import struct
import os

def create_mo_file(translations, output_path, plural_info=None):
    """
    Create a .mo file with the given translations.

    translations: dict of msgid -> msgstr (or list for plural forms)
    plural_info: tuple of (nplurals, plural_expr) for plural forms header
    """
    # Sort translations by msgid for binary search
    sorted_items = sorted(translations.items(), key=lambda x: x[0])

    # Create header entry (empty msgid)
    header_parts = []
    header_parts.append("Content-Type: text/plain; charset=UTF-8")
    header_parts.append("Content-Transfer-Encoding: 8bit")
    if plural_info:
        nplurals, plural_expr = plural_info
        header_parts.append(f"Plural-Forms: nplurals={nplurals}; plural={plural_expr};")
    header = "\\n".join(header_parts).encode('utf-8')

    # Prepare all strings
    originals = []  # (offset, length, bytes)
    translations_data = []  # (offset, length, bytes)

    # Add header first
    originals.append(b"")
    translations_data.append(header)

    # Add all translations
    for msgid, msgstr in sorted_items:
        if msgid == "":
            continue  # Skip empty, already added as header

        if isinstance(msgstr, list):
            # Plural forms: msgid\0msgid_plural for original
            # msgstr0\0msgstr1\0... for translation
            if isinstance(msgid, tuple):
                orig = msgid[0].encode('utf-8') + b'\\x00' + msgid[1].encode('utf-8')
            else:
                orig = msgid.encode('utf-8')
            trans = b'\\x00'.join(s.encode('utf-8') for s in msgstr)
        else:
            orig = msgid.encode('utf-8')
            trans = msgstr.encode('utf-8')

        originals.append(orig)
        translations_data.append(trans)

    N = len(originals)

    # Calculate offsets
    # Header: 28 bytes
    # Original string table: N * 8 bytes
    # Translation string table: N * 8 bytes
    # No hash table
    # Strings follow

    header_size = 28
    orig_table_offset = header_size
    trans_table_offset = orig_table_offset + N * 8
    strings_offset = trans_table_offset + N * 8

    # Calculate string offsets
    current_offset = strings_offset
    orig_descriptors = []
    for s in originals:
        orig_descriptors.append((len(s), current_offset))
        current_offset += len(s) + 1  # +1 for NUL

    trans_descriptors = []
    for s in translations_data:
        trans_descriptors.append((len(s), current_offset))
        current_offset += len(s) + 1  # +1 for NUL

    # Build the file
    data = bytearray()

    # Magic number (little endian)
    data += struct.pack('<I', 0x950412de)
    # Revision (0)
    data += struct.pack('<I', 0)
    # Number of strings
    data += struct.pack('<I', N)
    # Offset of original strings table
    data += struct.pack('<I', orig_table_offset)
    # Offset of translation strings table
    data += struct.pack('<I', trans_table_offset)
    # Hash table size (0 = no hash table)
    data += struct.pack('<I', 0)
    # Hash table offset (0)
    data += struct.pack('<I', 0)

    # Original strings table
    for length, offset in orig_descriptors:
        data += struct.pack('<II', length, offset)

    # Translation strings table
    for length, offset in trans_descriptors:
        data += struct.pack('<II', length, offset)

    # String data
    for s in originals:
        data += s + b'\\x00'
    for s in translations_data:
        data += s + b'\\x00'

    # Write file
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(data)

    print(f"Created: {output_path}")

def main():
    # Create test directory structure
    base_dir = os.path.dirname(os.path.abspath(__file__))
    locale_dir = os.path.join(base_dir, "test_locale")

    # German translations
    de_translations = {
        "Hello": "Hallo",
        "World": "Welt",
        "Hello World": "Hallo Welt",
        "Good morning": "Guten Morgen",
        "Thank you": "Danke",
        "Yes": "Ja",
        "No": "Nein",
    }

    # Create German .mo file
    de_path = os.path.join(locale_dir, "de", "LC_MESSAGES", "test.mo")
    create_mo_file(de_translations, de_path)

    # French translations
    fr_translations = {
        "Hello": "Bonjour",
        "World": "Monde",
        "Hello World": "Bonjour le monde",
        "Good morning": "Bonjour",
        "Thank you": "Merci",
        "Yes": "Oui",
        "No": "Non",
    }

    fr_path = os.path.join(locale_dir, "fr", "LC_MESSAGES", "test.mo")
    create_mo_file(fr_translations, fr_path)

    print("\\nTest .mo files created successfully!")
    print(f"Locale directory: {locale_dir}")

if __name__ == "__main__":
    main()
