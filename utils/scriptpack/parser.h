#include <iterator>
#include <algorithm>
#include <sstream>
#include <string>
#include <regex>
#include <ctype.h>
#include <stack>

class Parser
{
public:
  Parser(std::string &str) : m_Data(str){  }
  
  //credits to https://codereview.stackexchange.com/a/215913
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
            m_Result += '\n';
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
  
  //credits to https://www.tutorialspoint.com/cplusplus-program-to-check-for-balanced-paranthesis-by-using-stacks
  bool areBracketsClosed(char *filename)
  {
    std::stack<char> s;
    char ch;
    size_t line;
    
    for (int i = 0; i < m_Result.length(); i++) 
    { //for each character in the expression, check conditions
      if (m_Result[i] == '(' || m_Result[i] == '[' || m_Result[i] == '{') 
      { //when it is opening bracket, push into stack
        s.push(m_Result[i]);
        continue;
      }
      
      if (m_Result[i] == '\n')
        line++;
      
      if (s.empty()) //stack cannot be empty as it is not opening bracket, there must be closing bracket
        return false;
        
      switch (m_Result[i]) 
      {
        case ')': //for closing parenthesis, pop it and check for braces and square brackets
          ch = s.top();
          s.pop();
          if (ch == '{' || ch == '[')
          {
            closingError(filename, line);
            return false;
          }
          break;
        case '}': //for closing braces, pop it and check for parenthesis and square brackets
          ch = s.top();
          s.pop();
          if (ch == '(' || ch == '[')
          {
            closingError(filename, line);
            return false;
          }
          break;
        case ']': //for closing square bracket, pop it and check for braces and parenthesis
          ch = s.top();
          s.pop();
          if (ch == '(' || ch == '{')
          {
            closingError(filename, line);
            return false;
          }
          break;
      }
    }
    
    return (s.empty()); //when stack is empty, return true
  }
  
  std::string getResult()
  {
    return m_Result;
  }
  
  std::vector<std::string> getErrorlist()
  {
    return m_ErrorList;
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
  
  //file:line missing closing brackets
  void closingError(char *file, size_t line)
  {
    char errStr[256];
    std::snprintf(errStr, 0, "%s:%u missing bracket(s)\n", file, line);
    std::string s(errStr);
    m_ErrorList.push_back(s);
  }

	State m_cState = State::NotAComment;
  std::string m_Result{};
  std::string &m_Data;
  std::vector<std::string> m_ErrorList;
};