#include "rxui-qt.h"

#include <QtWidgets>

QtRoot::QtRoot(void *container) : container{container}
{
    this->size = sizeof(QtRoot);
    this->name = "QtRoot";
}

void QtRoot::init(rxui::Root &self, void *props)
{
    new(&self) QtRoot(props);
}

void QtRoot::push(void *it)
{
    auto obj = static_cast<QObject *>(it);
    if (auto layout = dynamic_cast<QLayout *>(obj)) {
        auto w = new QWidget;
        w->setLayout(layout);
        reinterpret_cast<QLayout *>(container)->addWidget(w);
    } else if (auto widget = dynamic_cast<QWidget *>(obj)) {
        reinterpret_cast<QLayout *>(container)->addWidget(widget);
    }
}

HBox::HBox() : handle{std::make_shared<QHBoxLayout>()}
{}

rxui::Element<> HBox::render()
{
    auto self = this;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

VBox::VBox() : handle{std::make_shared<QVBoxLayout>()}
{}

rxui::Element<> VBox::render()
{
    auto self = this;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Scrolled::Scrolled() : handle{std::make_shared<QVBoxLayout>()}
{}

rxui::Element<> Scrolled::render()
{
    auto self = this;
    return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
}

Button::Button() : handle{std::make_shared<QPushButton>()}
{
    auto self = this;
    auto handle = reinterpret_cast<QPushButton *>(self->handle.get());
    QObject::connect(handle, &QPushButton::clicked, [=]() {
        if (self->props->onClick) {
            self->props->onClick();
        }
    });
}

rxui::Element<> Button::render()
{
    auto self = this;
    auto handle = reinterpret_cast<QPushButton *>(self->handle.get());
    handle->setEnabled(!self->props->disabled);
    handle->setText(self->props->label.c_str());
    return rxui::intrinsic(self->handle);
}

Combo::Combo() : handle{std::make_shared<QComboBox>()}
{
    auto self = this;
    auto handle = reinterpret_cast<QComboBox *>(self->handle.get());
    QObject::connect(handle, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        auto value = handle->itemData(index).toString().toStdString();
        {
            QSignalBlocker blocker{handle};
            for (int i = handle->findData(self->props->selected.c_str()); i != -1;) {
                handle->setCurrentIndex(i);
                break;
            }
        }
        if (self->props->onChange) {
            self->props->onChange(value);
        }
    });
}

rxui::Element<> Combo::render()
{
    auto self = this;
    auto handle = reinterpret_cast<QComboBox *>(self->handle.get());

    {
        QSignalBlocker blocker{handle};
        for (auto &it : self->props->children) {
            handle->addItem(it.label.c_str(), it.value.c_str());
        }
        for (int i = handle->findData(self->props->selected.c_str()); i != -1;) {
            handle->setCurrentIndex(i);
            break;
        }
    }

    return rxui::intrinsic(self->handle);
}

Entry::Entry() : handle{std::make_shared<QLineEdit>()}
{
    auto self = this;
    auto handle = reinterpret_cast<QLineEdit *>(self->handle.get());

    QObject::connect(handle, &QLineEdit::textChanged, [=]() {
        auto s = handle->text().toStdString();
        {
            QSignalBlocker blocker{handle};
            handle->setText(self->props->text.c_str());
        }
        if (self->props->onChange) {
            self->props->onChange(s);
        }
    });
}

rxui::Element<> Entry::render()
{
    auto self = this;
    auto handle = reinterpret_cast<QLineEdit *>(self->handle.get());

    handle->setEnabled(!self->props->disabled);
    {
        QSignalBlocker blocker{handle};
        handle->setText(self->props->text.c_str());
    }

    QPalette palette;
    if (!self->props->background.empty()) {
        QColor color;
        color.setNamedColor(self->props->background.c_str());
        palette.setColor(QPalette::Base, color);
    }
    handle->setPalette(palette);
    return rxui::intrinsic(self->handle);
}

Label::Label() : handle{std::make_shared<QLabel>()}
{}

rxui::Element<> Label::render()
{
    auto self = this;
    auto handle = reinterpret_cast<QLabel *>(self->handle.get());
    handle->setText(self->props->text.c_str());
    return rxui::intrinsic(self->handle);
}

List::List() : handle{std::make_shared<QListWidget>()}
{
    auto self = this;
    auto handle = reinterpret_cast<QListWidget *>(self->handle.get());
    QObject::connect(handle, &QListWidget::currentRowChanged, [=](int row) {
        if (row < 0) {
            return;
        }
        int active = -1;
        int i = -1;
        for (auto &it : self->props->children) {
            i += 1;
            if (it.value == self->props->selected) {
                active = i;
                break;
            }
        }
        if (active >= 0) {
            QSignalBlocker blocker{handle};
            handle->setCurrentRow(active);
        }
        if (self->props->onChange) {
            auto i = handle->item(row)->data(Qt::UserRole).toInt();
            self->props->onChange(self->props->children[i].value);
        }
    });
}

rxui::Element<> List::render()
{
    auto self = this;
    auto handle = reinterpret_cast<QListWidget *>(self->handle.get());

    handle->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    int active = -1;
    int i = -1;
    for (auto &it : self->props->children) {
        i += 1;
        handle->addItem(it.label.c_str());
        handle->item(i)->setData(Qt::UserRole, i);
        if (active < 0 && it.value == self->props->selected) {
            active = i;
        }
    }
    if (active >= 0) {
        QSignalBlocker blocker{handle};
        handle->setCurrentRow(active);
    }

    return rxui::intrinsic(self->handle);
}


