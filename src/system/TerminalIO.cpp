#include <iostream>
#include <stdexcept>
#include <zg/system/TerminalIO.hpp>
using namespace zg::system;
TeIO::TeIO(bool outIsErr) :
#if defined(__linux__) || defined(__APPLE__) || defined(_WIN32)
		m_read(STDIN_FILENO), m_write(outIsErr ? STDERR_FILENO : STDOUT_FILENO),
#if defined(__linux__) || defined(__APPLE__)
		m_originalProfile(new termios())
#else
#endif
#endif
{
#if defined(__linux__) || defined(__APPLE__)
	tcgetattr(m_read, m_originalProfile);
#elif defined(_WIN32)
#endif
	m_currentProfile = new termios(*m_originalProfile);
}
TeIO::~TeIO()
{
#if defined(__linux__) || defined(__APPLE__)
	tcsetattr(m_read, TCSANOW, m_originalProfile);
	delete m_currentProfile;
	delete m_originalProfile;
#elif defined(_WIN32)
#endif
}
string TeIO::command(string_view cmd, char cmdEndChr)
{
	string strin_;
	write(m_write, cmd.data(), cmd.size());
	char byte;
	while (read(m_read, &byte, 1) == 1 && byte != cmdEndChr)
		strin_ += byte;
	return strin_;
}
glm::ivec2 TeIO::getDims()
{
	glm::ivec2 dims;
	auto resp = command("\033[18t", 't');
	if (sscanf(resp.c_str(), "\033[8;%d;%dt", &dims.x, &dims.y) == 2)
	{
		return dims;
	}
	return dims;
}
glm::ivec2 TeIO::getCursor()
{
	auto response = command("\033[6n", 'R');
    glm::ivec2 position({-1, -1});
	if (sscanf(response.c_str(), "\033[%d;%d", &position.x, &position.y) != 2)
		throw runtime_error("Invalid getCursor response!");
	return position;
}
bool TeIO::setCursor(glm::ivec2 pos)
{
	auto posx = to_string(pos.x);
	auto posy = to_string(pos.y);
	write(m_write, "\033[", 2);
	write(m_write, posy.c_str(), posy.size());
	write(m_write, ";", 1);
	write(m_write, posx.c_str(), posx.size());
	write(m_write, "H", 1);
	if (m_write == STDOUT_FILENO)
		cout << flush;
	else if (m_write == STDERR_FILENO)
		cerr << flush;
	return false;
}
void TeIO::echo(bool enable)
{
	if (enable)
		m_currentProfile->c_lflag &= (ECHO);
	else
		m_currentProfile->c_lflag &= ~(ECHO);
}
void TeIO::canonical(bool enable)
{
	if (enable)
		m_currentProfile->c_lflag &= (ICANON);
	else
		m_currentProfile->c_lflag &= ~(ICANON);
}
void TeIO::setProfile() { tcsetattr(m_read, TCSANOW, m_currentProfile); }