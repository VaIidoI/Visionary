#include <SFML/Graphics.hpp>
#include <iostream> 
#include <string>
#include <cstdint>
#include <algorithm>

constexpr uint64_t TEXT_SIZE = 30;

sf::Text& GetTextBase() {
    static bool firstCall = true;
    static sf::Font font;
    
    if(firstCall && !font.openFromFile("Fonts/anon.ttf"))
        throw std::runtime_error("Cannot load the font.");

    static sf::Text text(font, "", TEXT_SIZE);

    if(firstCall) {
        text.setFillColor(sf::Color::White);
        text.setPosition({0, 0});
        firstCall = false;
    }

    return text;
}

class LineManager {
public:
    LineManager() : m_Index(2), m_Text("Hello, World!\n"), m_CursorShape({2.0f, TEXT_SIZE}) {
        m_CursorShape.setFillColor(sf::Color::White);
    }

    void Draw(sf::RenderWindow& window) noexcept {
        sf::Text text(GetTextBase()); text.setString(m_Text);
        m_CursorShape.setPosition(text.findCharacterPos(m_Index));
        window.draw(text);
        window.draw(m_CursorShape);
    }

    void Update(double deltaTime) noexcept {

    }

    /**
     * Adds a character to the right of the cursor. 
     * If selecting, deletes the selected text.  
     * @param c The character to add. 
     * @throws std::out_of_range Thrown if the position of the cursor is out of bounds. 
     */
    void Add(char c)  {
        if(m_Text.size() < m_Index)
            throw std::out_of_range("Tried adding out of range.");
        m_Text.insert(m_Text.begin() + m_Index, c); 
        MoveRight();
    }

    /**
     * Removes a character to the left of the cursor. 
     * If selecting, deletes the selected text.  
     * @param c The character to add. 
     * @throws std::out_of_range Thrown if the position of the cursor is out of bounds. 
     */
    void Remove()  {
        // Do nothing if the cursor is on the first character.
        if(m_Index == 0)
            return;

        if(m_Text.size() < m_Index)
            throw std::out_of_range("Tried removing out of range.");

        m_Text.erase(m_Text.begin() + m_Index - 1); 
        MoveLeft();
    }

    bool MoveUp() {
        size_t nextLeftNewline = FindFirstLeftOfCursor('\n');
        if(nextLeftNewline == m_Text.size())
            return false;

        m_Index = nextLeftNewline;

        return true;
    }

    bool MoveDown() {
        size_t nextRightNewline = FindFirstRightOfCursor('\n');
        if(nextRightNewline == m_Text.size()) {
            return false;
        }

        m_Index = nextRightNewline;

        return true;
    }

    bool MoveLeft() {
        if(m_Index > 0) {
            m_Index--;
            return true;
        }

        return false;
    }

    bool MoveRight() {
        if(m_Index <= m_Text.size() - 1) {
            m_Index++;
            return true;
        }

        return false;
    }

    void SetBegin() {
        m_Index = 0;
    }

    void SetEnd() {
        if(m_Text.empty())
            return;

        m_Index = m_Text.size() - 1;
    }

private:
    size_t FindFirstLeftOfCursor(char c) {
        // If the cursor is on the first character, nothing is to the left of it.
        // Return the invalid pos.
        if(m_Index <= 0)
            return m_Text.size(); 

        for(auto it = m_Text.begin() + m_Index - 1; it >= m_Text.begin(); it--)
            if(*it == c)
                return std::distance(m_Text.begin(), it);

        return m_Text.size();
    }

    size_t FindFirstRightOfCursor(char c) {
        // If the cursor is on the last character, nothing is to the right of it.
        // Return the invalid pos.
        if(m_Index >= m_Text.size())
            return m_Text.size();   

        for(auto it = m_Text.begin() + m_Index + 1; it != m_Text.end(); it++)
            if(*it == c)
                return std::distance(m_Text.begin(), it);

        return m_Text.size();
    }

    sf::RectangleShape m_CursorShape;
    size_t m_Index; // The current position of the cursor.
    std::string m_Text;
};

class TextEditor {
public:
    TextEditor() : m_Lines() {}

    void Draw(sf::RenderWindow& window) noexcept {
        m_Lines.Draw(window);
    }

    void Update(double deltaTime) noexcept {
        m_Lines.Update(deltaTime);
    }

    void OnKeyPressed(const sf::Event::KeyPressed& keyPressedEvent) noexcept {
        auto key = keyPressedEvent.code;
		bool controlPressed = keyPressedEvent.control;
		bool shiftPressed = keyPressedEvent.shift;
        bool altPressed = keyPressedEvent.alt;

        if(key == sf::Keyboard::Key::Enter)
            m_Lines.Add('\n');

        if(key == sf::Keyboard::Key::Tab)
            m_Lines.Add('\t');

        if(key == sf::Keyboard::Key::Backspace)
            m_Lines.Remove();

        if(key == sf::Keyboard::Key::Left)
            m_Lines.MoveLeft();

        if(key == sf::Keyboard::Key::Right)
            m_Lines.MoveRight();

        if(key == sf::Keyboard::Key::Up)
            m_Lines.MoveUp();
        if(key == sf::Keyboard::Key::Down)
            m_Lines.MoveDown();
    }

    void OnTextEntered(const sf::Event::TextEntered& textEnteredEvent) noexcept {
        uint32_t unicode = textEnteredEvent.unicode;
		if (unicode >= 127 || unicode < 32)
			return;

        m_Lines.Add(static_cast<char>(unicode));
    }

private:
    LineManager m_Lines;
};

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "Visionary");
    window.setFramerateLimit(144);

    TextEditor editor;

    const auto onClose = [&window](const sf::Event::Closed& closedEvent) {
        window.close();
    };

    const auto onKeyPressed = [&editor](const sf::Event::KeyPressed& keyPressedEvent) {
		editor.OnKeyPressed(keyPressedEvent);
    };

    const auto onTextEntered = [&editor](const sf::Event::TextEntered& textEnteredEvent) {
        editor.OnTextEntered(textEnteredEvent);
    };

    while (window.isOpen())
    {
        window.handleEvents(onClose, onKeyPressed, onTextEntered);


        window.clear();
        editor.Draw(window);
        window.display();
    }

    return 0;
}
