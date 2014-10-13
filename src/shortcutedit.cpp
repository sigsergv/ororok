#include <QtGui>
#include <QtWidgets>

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QtDebug>

#include "shortcutedit.h"

struct ShortcutEdit::Private
{
    QLineEdit * lineedit;
    QPushButton * resetButton;

    bool emptySequenceReady;
    QList<QString> workingSequence;
    QKeySequence sequence;

    void resetCurrentShortcut();
    QString shortcutRepr();

    // modifiers
    bool finalized;
    int pressedKeys;
    bool ctrlMod;
    bool altMod;
    bool metaMod;
    bool shiftMod;
    int keyCode;
};

void ShortcutEdit::Private::resetCurrentShortcut()
{
    ctrlMod = altMod = metaMod = shiftMod = finalized = false;
    pressedKeys = keyCode = 0;
    lineedit->setText("");
}

QString ShortcutEdit::Private::shortcutRepr()
{
    QStringList s;

    if (shiftMod) {
        s << "Shift";
    }

    if (ctrlMod) {
        s << "Ctrl";
    }

    if (altMod) {
        s << "Alt";
    }

    if (metaMod) {
        s << "Meta";
    }

    if (keyCode) {
        QKeySequence seq(keyCode);
        s << seq.toString();
        QKeySequence newSeq(s.join("+"));

        sequence.swap(newSeq);
    }

    return s.join(" + ");
}

ShortcutEdit::ShortcutEdit(QWidget *parent) :
    QWidget(parent)
{
    p = new Private;
    p->emptySequenceReady = false;

    p->resetButton = new QPushButton(this);
    p->resetButton->setMaximumWidth(22);
    p->resetButton->setIcon(QIcon::fromTheme("edit-clear"));
    p->resetButton->setFocusPolicy(Qt::NoFocus);
    p->resetButton->setFlat(true);
    p->resetButton->hide();

    connect( p->resetButton, SIGNAL(clicked()),
            this, SLOT(resetClicked()) );

    QLayout * layout = new QHBoxLayout();
    p->lineedit = new QLineEdit(this);

    layout->addWidget(p->lineedit);
    layout->addWidget(p->resetButton);

    p->lineedit->installEventFilter(this);

    setLayout(layout);
}

ShortcutEdit::~ShortcutEdit()
{
    delete p;
}

void ShortcutEdit::handleKeyEvent(QKeyEvent * e)
{
    Qt::KeyboardModifiers state = e->modifiers();

    p->ctrlMod = state & Qt::ControlModifier;
    p->altMod = state & Qt::AltModifier;
    p->metaMod = state & Qt::MetaModifier;
    p->shiftMod = state & Qt::ShiftModifier;

    QString text = e->text();
    if (text.size() != 0) {
        // we are dealing with non-modifier key, so process it
        int key = e->key();
        p->keyCode = key;
        p->finalized = true;
    } else {
        p->keyCode = 0;
    }

    p->lineedit->setText(p->shortcutRepr());
    p->resetButton->show();
}

void ShortcutEdit::resetKeySequence()
{
    p->resetCurrentShortcut();
}

bool ShortcutEdit::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(e);
        if (!p->finalized) {
            handleKeyEvent(k);
        }
        p->pressedKeys++;
        return true;
    }

    if (e->type() == QEvent::Shortcut || e->type() == QEvent::KeyRelease) {
        QKeyEvent * k = static_cast<QKeyEvent*>(e);
        if (!p->finalized) {
            handleKeyEvent(k);
        }
        p->pressedKeys--;

        if (p->pressedKeys <= 0) {
            p->finalized = false;
        }
        return true;
    }

    if (e->type() == QEvent::ShortcutOverride) {
        //e->accept();
        return true;
    }

    if (e->type() == QEvent::FocusIn) {
        p->resetCurrentShortcut();
        p->emptySequenceReady = true;
    }

    if (e->type() == QEvent::FocusOut) {
        p->pressedKeys = 0;
        p->finalized = false;
    }

    return QObject::eventFilter(obj, e);
}

QKeySequence ShortcutEdit::keySequence() const
{
    return p->sequence;
}

void ShortcutEdit::setKeySequence(const QKeySequence & seq)
{
    QString repr = seq.toString();
    if (repr.isEmpty()) {
        p->resetButton->hide();
        return;
    }

    p->sequence = seq;
    p->lineedit->setText(repr);
    p->resetButton->show();
}

void ShortcutEdit::resetClicked()
{
    p->resetCurrentShortcut();
    p->resetButton->hide();
}
