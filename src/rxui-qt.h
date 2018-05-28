#pragma once

#include <functional>
#include <QtWidgets>

#include "rxui.h"

class QtRoot : public rxui::Root {
public:
    QLayout *container;

    explicit QtRoot(QLayout *container) : container{container}
    {
        this->size = sizeof(QtRoot);
        this->name = "QtRoot";
    }

    void init(Root &self, void *props) override
    {
        new(&self) QtRoot(static_cast<QLayout *>(props));
    }

    void push(void *it) override
    {
        auto obj = static_cast<QObject *>(it);
        if (auto layout = dynamic_cast<QLayout *>(obj)) {
            auto w = new QWidget;
            w->setLayout(layout);
            container->addWidget(w);
        } else if (auto widget = dynamic_cast<QWidget *>(obj)) {
            container->addWidget(widget);
        }
    }
};

struct HBoxProps {
};

class HBox : public rxui::Component<HBox, HBoxProps> {
    const std::shared_ptr<QHBoxLayout> handle{new QHBoxLayout};
public:
    rxui::Element<void> render() override
    {
        auto self = this;
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct VBoxProps {
};

class VBox : public rxui::Component<VBox, VBoxProps> {
    const std::shared_ptr<QVBoxLayout> handle{new QVBoxLayout{new QWidget}};
public:
    rxui::Element<void> render() override
    {
        auto self = this;
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct ScrolledProps {
};

class Scrolled : public rxui::Component<Scrolled, ScrolledProps> {
    const std::shared_ptr<QVBoxLayout> handle{new QVBoxLayout{new QWidget}};
public:
    rxui::Element<void> render() override
    {
        auto self = this;
        return rxui::intrinsic(self->handle, rxui::Flags::Portal | rxui::Flags::Fragment);
    }
};

struct ButtonProps {
    bool disabled;
    std::string label;
    std::function<void()> onClick;
};

class Button : public rxui::Component<Button, ButtonProps> {
    const std::shared_ptr<QPushButton> handle{new QPushButton};
public:
    Button()
    {
        auto self = this;
        auto handle = &*self->handle;
        QObject::connect(handle, &QPushButton::clicked, [=]() {
            if (self->props->onClick) {
                self->props->onClick();
            }
        });
    }

    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = &*self->handle;
        handle->setEnabled(!self->props->disabled);
        handle->setText(self->props->label.c_str());
        return rxui::intrinsic(self->handle);
    }
};

struct ComboProps {
    struct Option {
        std::string value;
        std::string label;
    };
    std::vector<Option> children;
    std::string selected;
    std::function<void(std::string)> onChange;
};

class Combo : public rxui::Component<Combo, ComboProps> {
    const std::shared_ptr<QComboBox> handle{new QComboBox};
public:
    Combo()
    {
        auto self = this;
        auto handle = &*self->handle;
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

    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = &*self->handle;

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
};

struct EntryProps {
    bool disabled;
    std::string text;
    std::function<void(std::string)> onChange;
    std::string background;
};

class Entry : public rxui::Component<Entry, EntryProps> {
    const std::shared_ptr<QLineEdit> handle{new QLineEdit};
public:
    Entry()
    {
        auto self = this;
        auto handle = &*self->handle;
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

    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = &*self->handle;
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
};

struct LabelProps {
    std::string text;
};

class Label : public rxui::Component<Label, LabelProps> {
    const std::shared_ptr<QLabel> handle{new QLabel};
public:
    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = &*self->handle;
        handle->setText(self->props->text.c_str());
        return rxui::intrinsic(self->handle);
    }
};

struct ListProps {
    struct Option {
        std::string value;
        std::string label;
    };
    std::vector<Option> children;
    std::string selected;
    std::function<void(std::string)> onChange;
};

class List : public rxui::Component<List, ListProps> {
    const std::shared_ptr<QListWidget> handle{new QListWidget};
public:
    List()
    {
        auto self = this;
        auto handle = &*self->handle;
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

    rxui::Element<void> render() override
    {
        auto self = this;
        auto handle = &*self->handle;

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
};
