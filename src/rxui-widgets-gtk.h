#pragma once

#include "rxui.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct HBoxProps {
};

class HBox : public NativeComponent<HBox, HBoxProps> {
public:
    const std::shared_ptr<void> handle;

    HBox();

    rxui::Element<> render() override;
};

struct VBoxProps {
};

class VBox : public NativeComponent<VBox, VBoxProps> {
public:
    const std::shared_ptr<void> handle;

    VBox();

    rxui::Element<> render() override;
};

struct ScrolledProps {
};

class Scrolled : public NativeComponent<Scrolled, ScrolledProps> {
public:
    const std::shared_ptr<void> handle;

    Scrolled();

    rxui::Element<> render() override;
};

struct ButtonProps {
    bool disabled;
    std::string label;
    std::function<void()> onClick;
};

class Button : public NativeComponent<Button, ButtonProps> {
public:
    const std::shared_ptr<void> handle;

    Button();

    rxui::Element<> render() override;
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

class Combo : public NativeComponent<Combo, ComboProps>, ComboState {
public:
    const std::shared_ptr<void> handle;

    Combo();

    rxui::Element<> render() override;
};

struct EntryProps {
    bool disabled;
    std::string text;
    std::function<void(std::string)> onChange;
    std::string background;
};

class Entry : public NativeComponent<Entry, EntryProps>, EntryState {
public:
    const std::shared_ptr<void> handle;

    Entry();

    rxui::Element<> render() override;
};

struct LabelProps {
    std::string text;
};

class Label : public NativeComponent<Label, LabelProps> {
public:
    const std::shared_ptr<void> handle;

    Label();

    rxui::Element<> render() override;
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

class List : public NativeComponent<List, ListProps>, ListState {
public:
    const std::shared_ptr<void> handle;

    List();

    rxui::Element<> render() override;
};
