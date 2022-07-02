#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>

class Parser
{
public:
  std::string stripComments(const std::string &str)
	{
		const auto size = str.length();
		for (auto idx = 0; idx < size; ++idx)
		{
			const auto ch = str[idx];
			switch (m_cState)
			{
				case State::SlashOC:
					if (ch == '/')
						m_cState = State::SingleLineComment;
					else
					{
						// ?????
						m_cState = State::NotAComment;
						m_Result += '/' + ch;
					}
					break;

				case State::SingleLineComment:
					if (ch == '\n')
						m_cState = State::NotAComment;
					break;
          
        case State::NotAComment:
					if (ch == '/')
						m_cState = State::SlashOC;
					else
						m_Result += ch;
					break;
			}
		}

		return m_Result;
	}

private:
	enum class State : char
	{
		SlashOC,
		SingleLineComment,
		NotAComment
	};

	State m_cState = State::NotAComment;
  std::string m_Result{};
};