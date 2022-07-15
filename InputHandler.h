#pragma once
class InputHandler
{
public:
	InputHandler() {}
	InputHandler(const InputHandler& other) {}
	~InputHandler() {}

	void Initialise() { for (int i = 0; i < 256; i++) m_keys[i] = false; }
	void KeyDown(unsigned int key) { m_keys[key] = true; }
	void KeyUp(unsigned int key) { m_keys[key] = false; }
	bool IsKeyDown(unsigned int key) { return m_keys[key]; }
	bool IsKeyUp(unsigned int key) { return !m_keys[key]; }

	bool m_keys[256];
};

