#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <QWidget>
#include <QKeySequence>

struct QEvent;
struct QKeyEvent;

class ShortcutEdit : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutEdit(QWidget *parent = 0);
    virtual ~ShortcutEdit();
    bool eventFilter(QObject *, QEvent *);
    QKeySequence keySequence() const;
    void setKeySequence(const QKeySequence &);

protected slots:
    void resetClicked();

protected:
    void handleKeyEvent(QKeyEvent*);
    void resetKeySequence();

private:
    struct Private;
    Private * p;
};

#endif // SHORTCUTEDIT_H
