# Notepad

Qt 6 Notepad application built with CMake.

## Implementation

The application uses `QMainWindow` with a central `QTextEdit`. File operations are implemented with `QFile` and `QTextStream`; open and save paths throw `NotepadFileException` and are handled with `try`/`catch` in the UI actions.

Spell checking loads `data/words.txt` into `std::set`. `SpellCheckerHighlighter` subclasses `QSyntaxHighlighter`, underlines misspelled words with `QTextCharFormat::SpellCheckUnderline`, and highlights common C++ keywords. The editor context menu adds up to five spelling suggestions for the word under the cursor. `Tools > Check Spelling` reports the first misspelled word and suggestions.

The status bar shows word count, line count, current line, and current column. Recent files are stored with `QSettings` and limited to five entries. Formatting includes bold, italic, underline, and text color selection. View actions support zoom in, zoom out, and reset.
