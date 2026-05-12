#pragma once

#include "spell_checker.h"
#include "spell_checker_highlighter.h"

#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QTextEdit>

#include <functional>

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void connectEditor();
    void loadDictionary();
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void openPath(const QString& path);
    void savePath(const QString& path);
    void setCurrentFile(const QString& path);
    void showError(const QString& message);
    void updateStatus();
    void updateWindowTitle();
    void updateRecentFiles();
    void addRecentFile(const QString& path);
    void showFindReplaceDialog();
    void findNext();
    void replaceOne();
    void replaceAll();
    void showWordFrequencyDialog();
    void applyTransform(const std::function<QString(const QString&)>& transform);
    void checkSpelling();
    void changeTextColor();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void showEditorContextMenu(const QPoint& position);
    QString wordUnderCursor(QTextCursor cursor) const;
    void replaceWordUnderCursor(const QString& replacement);

    QTextEdit* m_editor;
    QLabel* m_statusLabel;
    QString m_currentFile;
    QSettings m_settings;
    SpellChecker m_spellChecker;
    SpellCheckerHighlighter* m_highlighter;
    QDialog* m_findReplaceDialog;
    QLineEdit* m_findLineEdit;
    QLineEdit* m_replaceLineEdit;
    QCheckBox* m_caseSensitiveCheckBox;
    QMenu* m_recentFilesMenu;
    QList<QAction*> m_recentFileActions;
    int m_baseFontPointSize;
};
