#include "sfw/Theme.hpp"
#include "sfw/Gui.hpp"
#include <SFML/Graphics.hpp>

#include <iostream>
using namespace std;

sf::Color hex2color(const std::string& hexcolor)
{
    sf::Color color = sf::Color::Black;
    if (hexcolor.size() == 7) // #ffffff
    {
        color.r = (uint8_t)strtoul(hexcolor.substr(1, 2).c_str(), NULL, 16);
        color.g = (uint8_t)strtoul(hexcolor.substr(3, 2).c_str(), NULL, 16);
        color.b = (uint8_t)strtoul(hexcolor.substr(5, 2).c_str(), NULL, 16);
    }
    else if (hexcolor.size() == 4) // #fff
    {
        color.r = (uint8_t)strtoul(hexcolor.substr(1, 1).c_str(), NULL, 16) * 17;
        color.g = (uint8_t)strtoul(hexcolor.substr(2, 1).c_str(), NULL, 16) * 17;
        color.b = (uint8_t)strtoul(hexcolor.substr(3, 1).c_str(), NULL, 16) * 17;
    }
    return color;
}

struct Theme
{
    sf::Color backgroundColor;
    std::string texturePath;
};

//----------------------------------------------------------------------------
int main()
{
try {
    Theme defaultTheme = {
        hex2color("#dddbde"),
        "demo/texture-default.png"
    };

    Theme win98Theme = {
        hex2color("#d4d0c8"),
        "demo/texture-win98.png"
    };

    bool clear_bgnd = true;

    // Create the main window
    sf::RenderWindow window;
    window.create(sf::VideoMode({1024, 768}), "SFW Demo", sf::Style::Close|sf::Style::Resize);
    if (!window.isOpen()) {
        cerr << "- Failed to create SFML window!\n";
        return EXIT_FAILURE;
    }
    window.setFramerateLimit(30);

    gui::Menu demo(window);
    demo.setPosition(10, 10);

    gui::Theme::loadFont("demo/tahoma.ttf");
    gui::Theme::loadTexture(defaultTheme.texturePath);
    gui::Theme::textSize = 11;
    gui::Theme::click.textColor      = hex2color("#191B18");
    gui::Theme::click.textColorHover = hex2color("#191B18");
    gui::Theme::click.textColorFocus = hex2color("#000");
    gui::Theme::input.textColor = hex2color("#000");
    gui::Theme::input.textColorHover = hex2color("#000");
    gui::Theme::input.textColorFocus = hex2color("#000");
    gui::Theme::input.textSelectionColor = hex2color("#8791AD");
    gui::Theme::input.textPlaceholderColor = hex2color("#8791AD");
    gui::Theme::PADDING = 2.f;
    gui::Theme::windowBgColor = defaultTheme.backgroundColor;

    gui::HBoxLayout* hbox = demo.addHBoxLayout();
    gui::FormLayout* form = hbox->addFormLayout();

    sf::Text text("Hello world!", gui::Theme::getFont());
    text.setOrigin({text.getLocalBounds().width / 2, text.getLocalBounds().height / 2});
    text.setPosition({480, 240});

    // Textbox
    gui::TextBox* textbox = new gui::TextBox();
    textbox->setText("Hello world!");
    textbox->setCallback([&]() {
        text.setString(textbox->getText());
        text.setOrigin({text.getLocalBounds().width / 2, text.getLocalBounds().height / 2});
    });
    textbox->setPlaceholder("Type something!");
    form->addRow("Text", textbox);

    gui::TextBox* textbox2 = new gui::TextBox();
    textbox2->setText("Hello world!");
    textbox2->setMaxLength(5);
    form->addRow("Text with limit (5)", textbox2);

    // Slider + ProgressBar for rotation
    gui::Slider* sliderRotation = new gui::Slider();
    gui::ProgressBar* pbarRotation1 = new gui::ProgressBar(200.f, gui::Horizontal, gui::LabelNone);
    gui::ProgressBar* pbarRotation2 = new gui::ProgressBar(200.f, gui::Horizontal, gui::LabelOver);
    gui::ProgressBar* pbarRotation3 = new gui::ProgressBar(200.f, gui::Horizontal, gui::LabelOutside);

    sliderRotation->setStep(1);
    sliderRotation->setCallback([&]() {
        text.setRotation(sf::degrees(sliderRotation->getValue() * 360 / 100.f));
        pbarRotation1->setValue(sliderRotation->getValue());
        pbarRotation2->setValue(sliderRotation->getValue());
        pbarRotation3->setValue(sliderRotation->getValue());
    });
    form->addRow("Rotation", sliderRotation);

    // Slider + ProgressBar for scale
    gui::Slider* sliderScale = new gui::Slider();
    gui::ProgressBar* pbarScale1 = new gui::ProgressBar(100, gui::Vertical, gui::LabelNone);
    gui::ProgressBar* pbarScale2 = new gui::ProgressBar(100, gui::Vertical, gui::LabelOver);
    gui::ProgressBar* pbarScale3 = new gui::ProgressBar(100, gui::Vertical, gui::LabelOutside);
    sliderScale->setCallback([&]() {
        float scale = 1 + sliderScale->getValue() * 2 / 100.f;
        text.setScale({scale, scale});
        pbarScale1->setValue(sliderScale->getValue());
        pbarScale2->setValue(sliderScale->getValue());
        pbarScale3->setValue(sliderScale->getValue());
    });
    form->addRow("Scale", sliderScale);

    // OptionsBox for color
    gui::OptionsBox<sf::Color>* opt = new gui::OptionsBox<sf::Color>();
    opt->addItem("Red", sf::Color::Red);
    opt->addItem("Blue", sf::Color::Blue);
    opt->addItem("Green", sf::Color::Green);
    opt->addItem("Yellow", sf::Color::Yellow);
    opt->addItem("White", sf::Color::White);
    opt->setCallback([&]() {
        text.setFillColor(opt->getSelectedValue());
    });
    form->addRow("Text color", opt);

    auto* cloned_opt = new gui::OptionsBox<sf::Color>(*opt); //!! -> Issue #26!
    cloned_opt->setCallback([&]() {
	gui::Theme::windowBgColor = cloned_opt->getSelectedValue();
    });
    form->addRow("Bgnd. (via cloned OptionBox)", cloned_opt);

    // Checbkox
    gui::CheckBox* checkboxBold = new gui::CheckBox();
    checkboxBold->setCallback([&]() {
        int style = text.getStyle();
        if (checkboxBold->isChecked())
            style |= sf::Text::Bold;
        else
            style &= ~sf::Text::Bold;
        text.setStyle(style);
    });
    form->addRow("Bold text", checkboxBold);

    gui::CheckBox* checkboxUnderlined = new gui::CheckBox();
    checkboxUnderlined->setCallback([&]() {
        int style = text.getStyle();
        if (checkboxUnderlined->isChecked())
            style |= sf::Text::Underlined;
        else
            style &= ~sf::Text::Underlined;
        text.setStyle(style);
    });
    form->addRow("Underlined text", checkboxUnderlined);

    // Progress bar
    form->addRow("Progress bar (label = None)", pbarRotation1);
    form->addRow("Progress bar (label = Over)", pbarRotation2);
    form->addRow("Progress bar (label = Outside)", pbarRotation3);

    // Vertical progress bars
    gui::Layout* layoutForVerticalProgressBars = new gui::HBoxLayout();
    layoutForVerticalProgressBars->add(pbarScale1);
    layoutForVerticalProgressBars->add(pbarScale2);
    layoutForVerticalProgressBars->add(pbarScale3);
    form->addRow("Vertical progress bars", layoutForVerticalProgressBars);

    form->addRow("Default button", new gui::Button("button"));

    // Custom button
    sf::Texture buttonimg;
    if (!buttonimg.loadFromFile("demo/themed-button.png")) {
        cerr << "- Failed to load button theme image!\n";
    }
    gui::SpriteButton* customButton = new gui::SpriteButton(buttonimg, "Play");
    customButton->setTextSize(20);
    form->addRow("Custom button", customButton);

    gui::VBoxLayout* vbox = hbox->addVBoxLayout();
    vbox->addLabel("Change theme:"); //!!?? It used to say: "panel on the left"... But why? It's on the right!

    gui::OptionsBox<Theme>* themeBox = new gui::OptionsBox<Theme>();
    themeBox->addItem("Windows 98", win98Theme);
    themeBox->addItem("Default", defaultTheme);
    themeBox->setCallback([&]() {
        const Theme& theme = themeBox->getSelectedValue();
        gui::Theme::loadTexture(theme.texturePath);
        gui::Theme::windowBgColor = theme.backgroundColor;
    });
    vbox->add(themeBox);

    // Textbox
    gui::HBoxLayout* hbox2 = vbox->addHBoxLayout();
    gui::TextBox* textbox3 = new gui::TextBox(100);
    textbox3->setText("Edit Me!");
    textbox3->setPlaceholder("Button label");
    hbox2->add(textbox3);
    hbox2->addButton("Create button", [&]() {
        vbox->add(new gui::Button(textbox3->getText()));
    });

    // Small progress bar
    gui::Image* imgCrop = nullptr; // Hold your breath, see it created below!
    gui::HBoxLayout* hbox3 = vbox->addHBoxLayout();
    hbox3->addLabel("Crop square size:");
    gui::ProgressBar* pbar = new gui::ProgressBar(40);
    hbox3->add(pbar);

    gui::Slider* vslider = new gui::Slider(100, gui::Vertical);
    vslider->setCallback([&]() {
        cerr << "Slider value: " << vslider->getValue() << endl;
        pbar->setValue(vslider->getValue());
        imgCrop->setCropRect({{vslider->getValue() / 4, vslider->getValue() / 10},
                              {vslider->getValue(), vslider->getValue()}});
    });
    hbox->add(vslider);

    // Image
    gui::VBoxLayout* vboximg = hbox->addVBoxLayout();
    vboximg->add(new gui::Label("Image:"));
    vboximg->add(new gui::Image("demo/image.png"));
//!!ADD PROTECTION AGAINST THIS ERROR TO THE LAYOUT CODE:
//!!    hbox->add(vboximg);
    // Cropped Images
    vboximg->add(new gui::Label("Cropped image:"));
    vboximg->add(new gui::Image("demo/image.png", {{0, 33}, {24, 28}}));
    vboximg->add(new gui::Label("Cropped image, resized:"));
    vboximg->add(imgCrop = new gui::Image("demo/sfml.png"));

    // Clear-background checkbox
    gui::HBoxLayout* hbox4 = demo.addHBoxLayout();
    auto* checkboxBgnd = new gui::CheckBox(true);
    checkboxBgnd->setCallback([&]() {
        clear_bgnd = checkboxBgnd->isChecked();
    });
    hbox4->add(new gui::Label("Clear background"));
    hbox4->add(checkboxBgnd);

    demo.addButton("Quit", [&]() {
        window.close();
    });

/*
    // Just an sf::Sprite, unused yet...
    sf::Texture texture;
    if (!texture.loadFromFile("demo/sfml.png")) {
        cerr << "- Failed to load texture!\n";
    }
    sf::Sprite sprite(texture);
    sprite.setOrigin({(float)texture.getSize().x / 2.f, (float)texture.getSize().y / 2.f});
    sprite.setPosition({600, 360});
*/
    // Start the event loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Send events to demo
            demo.onEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear screen
	if (clear_bgnd) window.clear(gui::Theme::windowBgColor);
        window.draw(demo);
        window.draw(text);
        // Update the window
        window.display();
    }
} catch (...) {
    cerr << "- Unhandled exception!\n";
    return EXIT_FAILURE;
}
    return EXIT_SUCCESS;
}
