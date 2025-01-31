#include "sfw/GUI.hpp"

#include <SFML/Graphics.hpp>

#include <string> // to_string
#include <iostream> // cerr, for errors, cout for some "demo" info
#include <thread>
#include <chrono>
#include <cassert>
using namespace std;

void background_thread_main(sfw::GUI& gui);

int main()
{
	//------------------------------------------------------------------------
	// Normal SFML app-specific setup....

	// Create the main window, as usual for any SFML app
	sf::RenderWindow window;
	window.create(sf::VideoMode({1024, 768}), "SFW Demo", sf::Style::Close|sf::Style::Resize);
	if (!window.isOpen()) {
		cerr << "- Failed to create the SFML window!\n";
		return EXIT_FAILURE;
	}
	window.setFramerateLimit(30);

	//------------------------------------------------------------------------
	// Setting up the GUI...

	// Just to spare some noise & typing...
	using sfw::Theme, sfw::hex2color, sfw::VBox, sfw::HBox, sfw::Form;
	using OBColor = sfw::OptionsBox<sf::Color>;

	// Customizing some theme props. (optional)

	Theme::DEFAULT.basePath = "demo/";
	Theme::DEFAULT.fontFile = "font/Vera.ttf";

	Theme::PADDING = 2.f;
	Theme::click.textColor      = hex2color("#191B18");
	Theme::click.textColorHover = hex2color("#191B18");
	Theme::click.textColorFocus = hex2color("#000");
	Theme::input.textColor = hex2color("#000");
	Theme::input.textColorHover = hex2color("#000");
	Theme::input.textColorFocus = hex2color("#000");
	Theme::input.textSelectionColor = hex2color("#8791AD");
	Theme::input.textPlaceholderColor = hex2color("#8791AD");

	// Some dynamically switcahble theme "quick config packs" to play with
	Theme::Cfg themes[] = {
		{ "Default (\"Baseline\")", "demo/", "texture-sfw-baseline.png", hex2color("#e6e8e0"), 11, "font/Liberation/LiberationSans-Regular.ttf" },
		{ "Classic ☺",              "demo/", "texture-sfw-classic.png",  hex2color("#e6e8e0"), 12, "font/Liberation/LiberationSans-Regular.ttf" },
		{ "sfml-widgets's default", "demo/", "texture-sfmlwidgets-default.png", hex2color("#dddbde"), },
		{ "sfml-widgets's Win98",   "demo/", "texture-sfmlwidgets-win98.png",   hex2color("#d4d0c8"), },
	};
	const size_t DEFAULT_THEME = 0;

	//--------------------------------------------------------------------
	// Create the main GUI controller:
	sfw::GUI demo(window, themes[DEFAULT_THEME]);
	if (!demo) {
		return EXIT_FAILURE; // Errors have already been printed to cerr.
	}
	demo.setPosition(10, 10);

	//--------------------------------------------------------------------
	// Some raw, native SFML example shapes (sf::Text + a filled rect),
	// to be manipulated by other widgets...
	// + a wrapper widget for boxing them nicely

		sf::Text text("Hello world!", Theme::getFont());
		sf::RectangleShape textrect;

	auto sfText = new sfw::DrawHost([&](auto* raw_w, auto ctx) {
		// Get current (raw, untransformed) text size
		//! Note: the text string & style attributes may have been
		//! changed by other widgets!
		auto tbounds = text.getLocalBounds();
		//! sf::Text's bound-rect is not 0-based! :-o (-> SFML #216)
		sf::Vector2f mysterious_sfml_offset = {tbounds.left, tbounds.top};
		sf::Vector2f tsize = {tbounds.width + tbounds.left, tbounds.height + tbounds.top};
		// Get boundrect of the transformed (rotated, scaled etc.) text
		// for sizing the wrapper widget
		sf::FloatRect bgRect{{0,0}, {tsize.x * 1.2f, tsize.y * 2.75f}};
		auto actualBoundRect = text.getTransform().transformRect(bgRect);

		auto* w = (sfw::DrawHost*)raw_w;

		//!! Umm... This is *GROSS*! Basically triggers a full GUI resize, right from draw()! :-oo
		w->setSize((actualBoundRect.width  + 2 * Theme::PADDING),
		           (actualBoundRect.height + 2 * Theme::PADDING));

		//!! Must position the shapes after setSize, as setSize may move the widget
		//!! out from the content we manually hack here, leaving the appearance off
		//!! for the current frame! :-o (A very visible artifact, as I've learned!...)
		//!! Well, another disadvantage of calling setSize during draw()!...
		//!! Fortunately, this one can be tackled by doing this after setSize:

		// Sync the bg. rect to various "externalia":
		textrect.setScale(text.getScale());
		textrect.setRotation(text.getRotation());
		textrect.setSize(bgRect.getSize());
		textrect.setOrigin({textrect.getSize().x / 2, textrect.getSize().y / 2});
		textrect.setPosition({w->getSize().x / 2, w->getSize().y / 2});

		text.setOrigin({tsize.x / 2 + mysterious_sfml_offset.x,
                                tsize.y / 2 + mysterious_sfml_offset.y}); //! sf::Text's bound-rect is not 0-based! :-o
		text.setPosition({w->getSize().x / 2, w->getSize().y / 2});

		auto sfml_renderstates = ctx.props;
		sfml_renderstates.transform *= w->getTransform();
		ctx.target.draw(textrect, sfml_renderstates);
		ctx.target.draw(text, sfml_renderstates);
#ifdef DEBUG
		w->draw_outline(ctx);
#endif
	});


	//--------------------------------------------------------------------
	// A horizontal layout for multiple panels side-by-side
	auto main_hbox = demo.add(new HBox);


	//--------------------------------------------------------------------
	// A "form" panel on the left
	auto left_panel = main_hbox->add(new VBox);

	auto form = left_panel->add(new Form);

	// A text box to set the text of the example SFML object (created above)
	form->add("Text", new sfw::TextBox(175)) // <- width (pixels)
		->setPlaceholder("Type something!")
		->setText("Hello world!")
		->setCallback([&](auto* w) { text.setString(w->getText()); });
	// Another text box, with length limit & pulsating cursor
	form->add("Text with limit (5)", new sfw::TextBox(50.f, sfw::TextBox::CursorStyle::PULSE))
		->setMaxLength(5)
		->setText("Hello world!");

	// Slider + progress bar for scaling
	auto sliderForSize = new sfw::Slider(10, 175); // granularity (%), width (pixel)
	auto pbarScale = new sfw::ProgressBar(175, sfw::Horizontal, sfw::LabelOver);
	sliderForSize->setCallback([&](auto* w) {
		float scale = 1 + w->getValue() * 2 / 100.f;
		text.setScale({scale, scale});
		pbarScale->setValue(w->getValue());
	});
	// Slider + progress bar for rotating
	auto sliderForRotation = new sfw::Slider(1, 75, sfw::Vertical); // granularity: 1%
	auto pbarRotation = new sfw::ProgressBar(75.f, sfw::Vertical, sfw::LabelOver);
	sliderForRotation->setCallback([&](auto* w) {
		text.setRotation(sf::degrees(w->getValue() * 360 / 100.f));
		pbarRotation->setValue(w->getValue());
	});

	// Add the scaling slider + its horizontal progress bar
	form->add("Size", sliderForSize);
	form->add("", pbarScale);

	// Add the rotation slider + its vertical progress bar
        // ...And, since there would be too much wasted space next to the p.bar,
	// put some text-styling checkboxes there. :)
	// (Note the use of the "new-less" (move-constructing) add(...) style, for a change!)
	auto rot_and_chkboxes = form->add("Rotation", HBox());
		rot_and_chkboxes->add(sliderForRotation, "rotation-slider");
		rot_and_chkboxes->add(pbarRotation);
		rot_and_chkboxes->add(sfw::Label("         ")); // Just a (vert.) spacer...

		auto styleboxes = rot_and_chkboxes->add(Form());
		// Checkboxes to set text properties (using all 3 query styles):
		styleboxes->add("Bold text", sfw::CheckBox([&](auto* w) {
			int style = text.getStyle();
			if (*w) style |=  sf::Text::Bold;
			else    style &= ~sf::Text::Bold;
			text.setStyle(style);
		}));
		styleboxes->add("Italic", sfw::CheckBox([&](auto* w) {
			int style = text.getStyle();
			if (w->checked()) style |=  sf::Text::Italic;
			else              style &= ~sf::Text::Italic;
			text.setStyle(style);
		}));
		styleboxes->add("Underlined", sfw::CheckBox([&](auto* w) {
			int style = text.getStyle();
			if (w->isChecked()) style |=  sf::Text::Underlined;
			else                style &= ~sf::Text::Underlined;
			text.setStyle(style);
		}));

	// Color selectors
	// -- Creating one as a template to clone it later, because
	//    WIDGETS MUSTN'T BE COPIED AFTER HAVING BEEN ADDED TO THE GUI!
	OBColor ColorSelect_TEMPLATE; (&ColorSelect_TEMPLATE)
		->add("Black", sf::Color::Black)
		->add("Red", sf::Color::Red)
		->add("Green", sf::Color::Green)
		->add("Blue", sf::Color::Blue)
		->add("Cyan", sf::Color::Cyan)
		->add("Yellow", sf::Color::Yellow)
		->add("White", sf::Color::White)
	;

	// Text color selector
	auto optTxtColor = (new OBColor(ColorSelect_TEMPLATE))
		->setCallback([&](auto* w) {
			text.setFillColor(w->current());
			w->setTextColor(w->current());
		});

	// Text backgorund rect color
	auto optTxtBg = (new OBColor(ColorSelect_TEMPLATE))
		->add("Default", themes[DEFAULT_THEME].bgColor)
		->setCallback([&](auto* w) {
			textrect.setFillColor(w->current());
//			w->setTextColor(sf::Color::White);
			w->setFillColor(w->current() * sf::Color(255, 255, 255, 64));
		});

	// (See also another one for window bg. color selection!)

	form->add("Text color", optTxtColor);
	form->add("Box color", optTxtBg);


	//--------------------------------------------------------------------
	// Buttons...
	auto buttons_vbox = form->add("Buttons", sfw::VBox());

	// "Button factory" labeller textbox + creat button...
	auto buttfactory = buttons_vbox->add(new HBox);
	auto labeller = buttfactory->add(sfw::TextBox(100))->setText("Edit Me!")->setPlaceholder("Button label");
	buttfactory->add(sfw::Button("Create button", [&] {
		buttons_vbox->addAfter(buttfactory, new sfw::Button(labeller->getText()));
	}));

	// More buttons...
	auto buttons_form = buttons_vbox->add(sfw::Form());

	auto utf8button_tag = "UTF-8 by default:"; // We'll also use this text to find the button.
	buttons_form->add(utf8button_tag, sfw::Button("Ψ ≠ 99° ± β")); // Note: this source is already UTF-8 encoded!
	cout << "UTF-8 button text got back as: \"" << ((sfw::Button*)demo.getWidget(utf8button_tag))->getText() << "\"\n";

	// Bitmap buttons
	auto imgbuttons_form = left_panel->add(sfw::Form());
	sf::Texture buttonimg; //! DON'T put this texture inside the if() as a local temporary!... ;)
	if (buttonimg.loadFromFile("demo/sfmlwidgets-themed-button.png")) // SFML would print an error if failed
	{
		auto combined_labels = new VBox();
		combined_labels->add(sfw::Label("Img. button"));
		combined_labels->add(sfw::Label("- native size"));		

		imgbuttons_form->add(combined_labels, new sfw::ImageButton(buttonimg, "All defaults"))
			->setTextSize(20)
			->setCallback([]/*(auto* w)*/ { /*no-arg. compilation test*/ });

		imgbuttons_form->add("- resized (etc.)", new sfw::ImageButton(buttonimg, "Bold"))
			->setTextSize(20)
			->setTextStyle(sf::Text::Style::Bold)
			->setSize({180, 35})
			->setTextColor(hex2color("#d0e0c0"))
			->setCallback([]/*(auto* w)*/ { /*no-arg. compilation test*/ });
	}


	//--------------------------------------------------------------------
	// A panel in the middle
	auto middle_panel = main_hbox->add(new VBox);

	// OK, add the SFML test shapes + adapter widget here
	middle_panel->add(sfText);


	//--------------------------------------------------------------------
	// Image views...

	// Image directly from file
	auto vboximg = main_hbox->add(new VBox);
	vboximg->add(sfw::Label("Image (directly from file):"));
	vboximg->add(sfw::Image("demo/thumbnail.jpg"));

	// Image crop + scaling directly from file
	vboximg->add(sfw::Label("Image crop + zoom:"));
	vboximg->add(sfw::Image("demo/thumbnail.jpg", {{100, 110}, {30, 20}}))->scale(3);

	// Image from file, cropped dynamically
	vboximg->add(sfw::Label("Dynamic cropping:"));

	sfw::Image* imgCrop = new sfw::Image("demo/martinet-dragonfly.jpg");
	// Slider & progress bar for cropping an Image widget
	auto cropslider = (new sfw::Slider(1, 100))->setCallback([&](auto* w) {
		((sfw::ProgressBar*)w->getWidget("cropbar"))->setValue(w->getValue());
		// Show the slider value in a text box retrieved by its name:
		auto tbox = (sfw::TextBox*)w->getWidget("crop%");
		if (!tbox) cerr << "Named TextBox not found! :-o\n";
		else tbox->setText(w->getValue() ? to_string((int)w->getValue()) : "");
		imgCrop->setCropRect({{(int)(w->getValue() / 4), (int)(w->getValue() / 10)},
		                      {(int)(w->getValue() * 1.4), (int)w->getValue()}});
	});

	vboximg->add(cropslider);
	auto boxcrop = vboximg->add(new HBox);
		boxcrop->add(sfw::Label("Crop square size:"));
		boxcrop->add(sfw::ProgressBar(40), "cropbar");
		boxcrop->add((new sfw::TextBox(36.f))->setMaxLength(3), "crop%")->setCallback([&](auto* w) {
			cropslider->setValue(stof(SFMLString_to_stdstring(w->getText())));
		});

	vboximg->add(imgCrop);
	vboximg->add(sfw::Label("(Art: © Édouard Martinet)"))->setStyle(sf::Text::Style::Italic);


	//--------------------------------------------------------------------
	// Another "sidebar" column, for (theme) introspection...
	auto right_bar = main_hbox->add(new VBox);

	// Theme selection
	using OBTheme = sfw::OptionsBox<Theme::Cfg>;
	right_bar->add(sfw::Label("Change theme:"));
	auto themeselect = new OBTheme([&](auto* w) {
		const auto& themecfg = w->current();
		demo.setTheme(themecfg); // Swallowing the error for YOLO reasons ;)
	});
	for (auto& t: themes) { themeselect->add(t.name, t); }
	themeselect->select(DEFAULT_THEME);
	right_bar->add(themeselect, "theme-selector");

	// Theme font size slider
	// (Changes the font size directly of the cfg. data stored in "theme-selector",
	// so it will remember the new size(s)!)
	right_bar->add(sfw::Label("Theme font size (use the m. wheel):"));
	right_bar->add(sfw::Slider(10, 100))
		->setValue(30)
		->setCallback([&] (auto* w){
			assert(w->getWidget("theme-selector"));
			auto& themecfg = ((OBTheme*)(w->getWidget("theme-selector")))->currentRef();
			themecfg.textSize = 8 + size_t(w->getValue() / 10);
//cerr << "font size: "<< themecfg.textSize << endl; //!!#196
			demo.setTheme(themecfg); // Forcing an onThemeChanged callback sweep...
		});

	// Show the current theme texture bitmaps
	right_bar->add(sfw::Label("Theme textures:"));
	auto txbox = right_bar->add(new HBox);
	struct ThemeBitmap : public sfw::Image {
		ThemeBitmap(float zoom) : Image(Theme::getTexture()) { scale(zoom); }
		void onThemeChanged() override {
			setTexture(Theme::getTexture()); // note: e.g. the ARROW is at {{0, 42}, {6, 6}}
		}
	};
	auto themeBitmap = new ThemeBitmap(2); // start with 2x zoom
	txbox->add(sfw::Slider(1.f, 100.f, sfw::Vertical))
		->setCallback([&](auto* w) { themeBitmap->scale(1 + (100.f - w->getValue()) / 25.f); })
		->setStep(25.f)
		->setValue(75.f);
	txbox->add(themeBitmap);

	right_bar->add(sfw::Label(" ")); // Just for some space, indeed...

	// A pretty useless, but interesting clear-background checkbox
	auto hbox4 = right_bar->add(new sfw::HBox);
	hbox4->add(sfw::Label("Clear background"));
	hbox4->add(sfw::CheckBox([&](auto* w) { Theme::clearBackground = w->checked(); }, true));

	// Window background color selector
	right_bar->add(new Form)->add("Bg. color", (new OBColor(ColorSelect_TEMPLATE))
		->add("Default", themes[DEFAULT_THEME].bgColor)
		->setCallback([&](auto* w) {
			Theme::bgColor = w->current();
		})
	);

	// Custom exit button (also useless, but feels so nice! :) )
	demo.add(sfw::Button("Quit", [&] { demo.close(); }));


	//--------------------------------------------------------------------
	// OK, GUI Setup done, set some "high-level" defaults
	// (after setup, as these may trigger callbacks etc.)
	//
	sliderForRotation->setValue(29);
	sliderForSize->setValue(20);
	// Colors of the example text + rect:
	optTxtColor->select("Red");
	optTxtBg->select("Black");

	//--------------------------------------------------------------------
	// Start another thread for the fun of it
	thread bg_thread(background_thread_main, std::ref(demo));

	//--------------------------------------------------------------------
	// Start the event loop
	while (window.isOpen())
	{
		// Render the GUI
		demo.render();

		// Show the updated window
		window.display();

		// Pass events to the GUI & check for closing or failure
		sf::Event event;
		while (window.pollEvent(event) && demo.process(event) )
		{
			// Just for convenience, close on Esc, too:
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				window.close(); // 
		}
	}

	//--------------------------------------------------------------------
	// Finish the bg. thread, too
	bg_thread.join();

	return EXIT_SUCCESS;
}


//----------------------------------------------------------------------------
void background_thread_main([[maybe_unused]] sfw::GUI& gui)
{
	auto sampletext_angle = sf::degrees(0);
	int n = 0;
	while (gui)
	{
	        this_thread::sleep_for(chrono::milliseconds(50));
		++n;
		sampletext_angle = sf::degrees(n*2.f);
		if (auto rot_slider = (sfw::Slider*)gui.getWidget("rotation-slider"); rot_slider)
		{
			rot_slider->setValue(float(int(sampletext_angle.asDegrees()/3) % 100));
		}
	}
}
