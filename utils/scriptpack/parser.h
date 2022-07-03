#include <iterator>
#include <algorithm>
#include <sstream>
#include <string>
#include <regex>
#include <ctype.h>

class Parser
{
public:
  Parser(std::string &str) : m_Data(str){  }
  
  void stripComments()
	{
    m_cState = State::NotAComment;
		const auto size = m_Data.length();
    
		for (auto idx = 0; idx < size; ++idx)
		{
			const auto ch = m_Data[idx];
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
	}
  
  void stripTabs()
  {
    //remove tabs
    m_Result.erase(std::remove(m_Result.begin(), m_Result.end(), '\t'), m_Result.end());
    
    //remove double spaces
    std::string::size_type pos = m_Result.find("  ");
    while (pos != std::string::npos)
    {
      m_Result.erase(pos, 2);
      pos = m_Result.find("  ", pos);
    }
  }
  
  void stripDebug()
  {
    std::regex re(R"(dbg (".*?"))");
    m_Result = std::regex_replace(m_Result, re, "");
  }
  
  void stripEmptyLines()
  {
    std::istringstream ss(m_Result);
    std::string line;
    std::string newRes = "";
    while(getline(ss, line))
    {
      //check if line is empty or has whitespace
      if (!line.empty() && !onlySpace(line))
      {
        newRes += line + "\n";
      }
    }
    
    m_Result = newRes;
  }
  
  std::string getResult()
  {
    return m_Result;
  }

private:
	enum class State : char
	{
		SlashOC,
		SingleLineComment,
		NotAComment,
	};
  
  bool onlySpace(const std::string &str)
  {
    return std::all_of(str.begin(),str.end(),isspace);
  }

	State m_cState = State::NotAComment;
  std::string m_Result{};
  std::string &m_Data;
};