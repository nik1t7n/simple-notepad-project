#include "main_window.h"

#include "notepad_exception.h"
#include "text_transform.h"
#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>

#include <map>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_editor(new QTextEdit(this))
    , m_statusLabel(new QLabel(this))
    , m_settings("OOPProject", "Notepad")
    , m_highlighter(nullptr)
    , m_findReplaceDialog(nullptr)
    , m_findLineEdit(nullptr)
    , m_replaceLineEdit(nullptr)
    , m_caseSensitiveCheckBox(nullptr)
    , m_recentFilesMenu(nullptr)
    , m_baseFontPointSize(11)
{
    setCentralWidget(m_editor);
    resize(900, 650);
    QFont font("DejaVu Sans Mono");
    font.setPointSize(m_baseFontPointSize);
    m_editor->setFont(font);
    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);

    loadDictionary();
    m_highlighter = new SpellCheckerHighlighter(m_editor->document(), &m_spellChecker);
    createActions();
    createMenus();
    createToolBar();
    connectEditor();
    statusBar()->addPermanentWidget(m_statusLabel);
    updateStatus();
    updateWindowTitle();
}

void MainWindow::createActions()
{
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("New", QKeySequence::New, this, [this] { newFile(); });
    fileMenu->addAction("Open", QKeySequence::Open, this, [this] { openFile(); });
    fileMenu->addAction("Save", QKeySequence::Save, this, [this] { saveFile(); });
    fileMenu->addAction("Save As", QKeySequence::SaveAs, this, [this] { saveFileAs(); });
    m_recentFilesMenu = fileMenu->addMenu("Recent Files");
    updateRecentFiles();
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, [this] { close(); });

    QMenu* editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction("Undo", QKeySequence::Undo, m_editor, &QTextEdit::undo);
    editMenu->addAction("Redo", QKeySequence::Redo, m_editor, &QTextEdit::redo);
    editMenu->addSeparator();
    editMenu->addAction("Cut", QKeySequence::Cut, m_editor, &QTextEdit::cut);
    editMenu->addAction("Copy", QKeySequence::Copy, m_editor, &QTextEdit::copy);
    editMenu->addAction("Paste", QKeySequence::Paste, m_editor, &QTextEdit::paste);
    editMenu->addAction("Select All", QKeySequence::SelectAll, m_editor, &QTextEdit::selectAll);
    editMenu->addSeparator();
    editMenu->addAction("Find / Replace", QKeySequence::Find, this, [this] { showFindReplaceDialog(); });
    editMenu->addAction("Word Frequency", this, [this] { showWordFrequencyDialog(); });

    QMenu* transformMenu = editMenu->addMenu("Text Transforms");
    transformMenu->addAction("Uppercase", this, [this] { applyTransform(TextTransform::uppercase); });
    transformMenu->addAction("Lowercase", this, [this] { applyTransform(TextTransform::lowercase); });
    transformMenu->addAction("Capitalize", this, [this] { applyTransform(TextTransform::capitalize); });
    transformMenu->addAction("Sentence Case", this, [this] { applyTransform(TextTransform::sentenceCase); });
    transformMenu->addAction("Swap Case", this, [this] { applyTransform(TextTransform::swapCase); });

    QMenu* formatMenu = menuBar()->addMenu("Format");
    formatMenu->addAction("Bold", QKeySequence::Bold, this, [this] {
        QTextCharFormat format;
        format.setFontWeight(m_editor->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
        m_editor->mergeCurrentCharFormat(format);
    });
    formatMenu->addAction("Italic", QKeySequence::Italic, this, [this] {
        QTextCharFormat format;
        format.setFontItalic(!m_editor->fontItalic());
        m_editor->mergeCurrentCharFormat(format);
    });
    formatMenu->addAction("Underline", QKeySequence::Underline, this, [this] {
        QTextCharFormat format;
        format.setFontUnderline(!m_editor->fontUnderline());
        m_editor->mergeCurrentCharFormat(format);
    });
    formatMenu->addAction("Text Color", this, [this] { changeTextColor(); });

    QMenu* viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction("Zoom In", QKeySequence::ZoomIn, this, [this] { zoomIn(); });
    viewMenu->addAction("Zoom Out", QKeySequence::ZoomOut, this, [this] { zoomOut(); });
    viewMenu->addAction("Reset Zoom", QKeySequence(Qt::CTRL | Qt::Key_0), this, [this] { zoomReset(); });

    QMenu* toolsMenu = menuBar()->addMenu("Tools");
    toolsMenu->addAction("Check Spelling", this, [this] { checkSpelling(); });
}

void MainWindow::createToolBar()
{
    QToolBar* toolbar = addToolBar("Format");
    toolbar->addAction(QIcon("data/images/bold.svg"), "Bold", this, [this] {
        QTextCharFormat format;
        format.setFontWeight(m_editor->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
        m_editor->mergeCurrentCharFormat(format);
    });
    toolbar->addAction(QIcon("data/images/italic.svg"), "Italic", this, [this] {
        QTextCharFormat format;
        format.setFontItalic(!m_editor->fontItalic());
        m_editor->mergeCurrentCharFormat(format);
    });
    toolbar->addAction(QIcon("data/images/underline.svg"), "Underline", this, [this] {
        QTextCharFormat format;
        format.setFontUnderline(!m_editor->fontUnderline());
        m_editor->mergeCurrentCharFormat(format);
    });
}

void MainWindow::connectEditor()
{
    connect(m_editor, &QTextEdit::textChanged, this, [this] { updateStatus(); });
    connect(m_editor, &QTextEdit::cursorPositionChanged, this, [this] { updateStatus(); });
    connect(m_editor, &QTextEdit::customContextMenuRequested, this, [this](const QPoint& position) { showEditorContextMenu(position); });
}

void MainWindow::loadDictionary()
{
    try {
        m_spellChecker.loadDictionary("data/words.txt");
    } catch (const notepad_exception& exception) {
        showError(QString::fromStdString(exception.what()));
    }
}

void MainWindow::newFile()
{
    m_editor->clear();
    setCurrentFile({});
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Open File");
    if (path.isEmpty())
        return;

    try {
        openPath(path);
    } catch (const notepad_exception& exception) {
        showError(QString::fromStdString(exception.what()));
    }
}

void MainWindow::saveFile()
{
    try {
        if (m_currentFile.isEmpty())
            saveFileAs();
        else
            savePath(m_currentFile);
    } catch (const notepad_exception& exception) {
        showError(QString::fromStdString(exception.what()));
    }
}

void MainWindow::saveFileAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save File");
    if (path.isEmpty())
        return;

    try {
        savePath(path);
    } catch (const notepad_exception& exception) {
        showError(QString::fromStdString(exception.what()));
    }
}

void MainWindow::openPath(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw file_read_exception(("Cannot open file: " + path).toStdString());

    QTextStream stream(&file);
    m_editor->setPlainText(stream.readAll());
    setCurrentFile(path);
    addRecentFile(path);
}

void MainWindow::savePath(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw file_write_exception(("Cannot save file: " + path).toStdString());

    QTextStream stream(&file);
    stream << m_editor->toPlainText();
    setCurrentFile(path);
    addRecentFile(path);
}

void MainWindow::setCurrentFile(const QString& path)
{
    m_currentFile = path;
    updateWindowTitle();
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::updateStatus()
{
    QString text = m_editor->toPlainText();
    int words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    int lines = m_editor->document()->blockCount();
    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.positionInBlock() + 1;
    m_statusLabel->setText(QString("Words: %1  Lines: %2  Line: %3  Column: %4").arg(words).arg(lines).arg(line).arg(column));
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(m_currentFile.isEmpty() ? "Notepad" : "Notepad: " + m_currentFile);
}

void MainWindow::updateRecentFiles()
{
    m_recentFilesMenu->clear();
    m_recentFileActions.clear();
    QStringList files = m_settings.value("recentFiles").toStringList();
    for (const QString& path : files) {
        QAction* action = m_recentFilesMenu->addAction(path, this, [this, path] {
            try {
                openPath(path);
            } catch (const notepad_exception& exception) {
                showError(QString::fromStdString(exception.what()));
            }
        });
        m_recentFileActions.append(action);
    }
    if (files.isEmpty())
        m_recentFilesMenu->addAction("Empty")->setEnabled(false);
}

void MainWindow::addRecentFile(const QString& path)
{
    QStringList files = m_settings.value("recentFiles").toStringList();
    files.removeAll(path);
    files.prepend(path);
    while (files.size() > 5)
        files.removeLast();
    m_settings.setValue("recentFiles", files);
    updateRecentFiles();
}

void MainWindow::showFindReplaceDialog()
{
    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new QDialog(this);
        Ui::FindReplaceDialog ui;
        ui.setupUi(m_findReplaceDialog);
        m_findLineEdit = m_findReplaceDialog->findChild<QLineEdit*>("findLineEdit");
        m_replaceLineEdit = m_findReplaceDialog->findChild<QLineEdit*>("replaceLineEdit");
        m_caseSensitiveCheckBox = m_findReplaceDialog->findChild<QCheckBox*>("caseSensitiveCheckBox");
        connect(m_findReplaceDialog->findChild<QPushButton*>("findNextButton"), &QPushButton::clicked, this, [this] { findNext(); });
        connect(m_findReplaceDialog->findChild<QPushButton*>("replaceButton"), &QPushButton::clicked, this, [this] { replaceOne(); });
        connect(m_findReplaceDialog->findChild<QPushButton*>("replaceAllButton"), &QPushButton::clicked, this, [this] { replaceAll(); });
        connect(m_findReplaceDialog->findChild<QPushButton*>("closeButton"), &QPushButton::clicked, m_findReplaceDialog, &QDialog::close);
    }
    m_findReplaceDialog->show();
    m_findReplaceDialog->raise();
    m_findReplaceDialog->activateWindow();
}

void MainWindow::findNext()
{
    QString term = m_findLineEdit ? m_findLineEdit->text() : QString();
    if (term.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox && m_caseSensitiveCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    if (!m_editor->find(term, flags)) {
        QTextCursor cursor = m_editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        m_editor->setTextCursor(cursor);
        m_editor->find(term, flags);
    }
}

void MainWindow::replaceOne()
{
    if (!m_findLineEdit || !m_replaceLineEdit)
        return;

    QTextCursor cursor = m_editor->textCursor();
    Qt::CaseSensitivity sensitivity = m_caseSensitiveCheckBox && m_caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (cursor.hasSelection() && cursor.selectedText().compare(m_findLineEdit->text(), sensitivity) == 0)
        cursor.insertText(m_replaceLineEdit->text());
    findNext();
}

void MainWindow::replaceAll()
{
    if (!m_findLineEdit || !m_replaceLineEdit || m_findLineEdit->text().isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox && m_caseSensitiveCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(cursor);
    while (m_editor->find(m_findLineEdit->text(), flags)) {
        QTextCursor found = m_editor->textCursor();
        found.insertText(m_replaceLineEdit->text());
    }
    cursor.endEditBlock();
}

void MainWindow::showWordFrequencyDialog()
{
    QDialog dialog(this);
    Ui::WordFrequencyDialog ui;
    ui.setupUi(&dialog);

    std::map<QString, int> frequencies;
    QRegularExpression expression("\\b[\\p{L}\\p{N}']+\\b");
    QRegularExpressionMatchIterator iterator = expression.globalMatch(m_editor->toPlainText().toLower());
    while (iterator.hasNext())
        ++frequencies[iterator.next().captured(0)];

    ui.frequencyTableWidget->setRowCount(static_cast<int>(frequencies.size()));
    int row = 0;
    for (const auto& entry : frequencies) {
        ui.frequencyTableWidget->setItem(row, 0, new QTableWidgetItem(entry.first));
        ui.frequencyTableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(entry.second)));
        ++row;
    }
    ui.frequencyTableWidget->resizeColumnsToContents();
    connect(ui.closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::applyTransform(const std::function<QString(const QString&)>& transform)
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText(transform(cursor.selectedText()));
    } else {
        QString transformed = transform(m_editor->toPlainText());
        m_editor->setPlainText(transformed);
    }
}

void MainWindow::checkSpelling()
{
    m_highlighter->rehighlight();
    QString word = m_spellChecker.firstMisspelled(m_editor->toPlainText());
    if (word.isEmpty()) {
        QMessageBox::information(this, "Check Spelling", "No spelling errors found.");
        return;
    }

    QStringList suggestions = m_spellChecker.suggestions(word, 5);
    QMessageBox::information(this, "Check Spelling", "First misspelled word: " + word + "\nSuggestions: " + suggestions.join(", "));
}

void MainWindow::changeTextColor()
{
    QColor color = QColorDialog::getColor(m_editor->textColor(), this, "Text Color");
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    m_editor->mergeCurrentCharFormat(format);
}

void MainWindow::zoomIn()
{
    m_editor->zoomIn(1);
}

void MainWindow::zoomOut()
{
    m_editor->zoomOut(1);
}

void MainWindow::zoomReset()
{
    QFont font = m_editor->font();
    font.setPointSize(m_baseFontPointSize);
    m_editor->setFont(font);
}

void MainWindow::showEditorContextMenu(const QPoint& position)
{
    QTextCursor cursor = m_editor->cursorForPosition(position);
    m_editor->setTextCursor(cursor);
    QString word = wordUnderCursor(cursor);
    QMenu* menu = m_editor->createStandardContextMenu();
    if (!word.isEmpty() && !m_spellChecker.isCorrect(word)) {
        menu->insertSeparator(menu->actions().isEmpty() ? nullptr : menu->actions().first());
        QStringList suggestions = m_spellChecker.suggestions(word, 5);
        for (const QString& suggestion : suggestions) {
            QAction* action = new QAction(suggestion, menu);
            connect(action, &QAction::triggered, this, [this, suggestion] { replaceWordUnderCursor(suggestion); });
            menu->insertAction(menu->actions().isEmpty() ? nullptr : menu->actions().first(), action);
        }
    }
    menu->exec(m_editor->mapToGlobal(position));
    delete menu;
}

QString MainWindow::wordUnderCursor(QTextCursor cursor) const
{
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void MainWindow::replaceWordUnderCursor(const QString& replacement)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(replacement);
}
