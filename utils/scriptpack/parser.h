#ifndef PARSER_H
#define PARSER_H

#include <iterator>
#include <sstream>
#include <regex>
#include <ctype.h>
#include <stack>

#include "cbase.h"

class Parser
{
public:
  Parser(std::string &data, char *file) : m_Data(data) {
    m_FileName = file;
  }
  
  //credits to https://codereview.stackexchange.com/a/215913
  void stripComments()
	{
    State cState = State::NotAComment;
		const size_t size = m_Data.length();
    
		for (size_t idx = 0; idx < size; ++idx)
		{
			const char ch = m_Data[idx];
			switch (cState)
			{
			case State::SlashOC:
				if (ch == '/')
					cState = State::SingleLineComment;
				else
				{
					// ?????
					cState = State::NotAComment;
					m_Result += '/' + ch;
				}
				break;

			case State::SingleLineComment:
				if (ch == '\n')
					cState = State::NotAComment;
          m_Result += '\n';
				break;
        
      case State::NotAComment:
				if (ch == '/')
					cState = State::SlashOC;
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
  
  bool checkClosing()
  { 
    //best to go off via original data.
    const size_t size = m_Data.length();
    size_t line = 1;
    
    if (size <= 0)
      return false;
    
    int oPara, cPara, oBrac, cBrac = 0;
    
    for (size_t i = 0; i < size; ++i) 
    {
      const char ch = m_Data[i];
      
      switch(ch)
      {
      case '\n':
        line++;
        break;
      case '(':
        oPara++;
        break;
      case ')':
        cPara++;
        break;
      case '{':
        oBrac++;
        break;
      case '}':
        cBrac++;
        break;
      }
    }
    
    if ((oPara != cPara) || (oBrac != cBrac))
    {
      closingError();
      return false;
    }
      
    return true;
  }
  
  bool checkQuotes()
  {
    State cState = State::NoQuote;
		const size_t size = m_Data.length();
    size_t line = 1;
    
    for (size_t i = 0; i < size; ++i)
    {
      const char ch = m_Data[i];
      
      if (ch == '\n')
        line++;
      
      switch(cState)
      {
        case State::InDoubleQuote:
          if (ch == '"')
            return true;
          else
          {
            quoteError(line);
            return false;
          }
          break;
        case State::InSingleQuote:
          if (ch == '\'')
            return true;
          else
          {
            quoteError(line);
            return false;
          }
          break;
        case State::NoQuote:
          if (ch == '"')
            cState = State::InDoubleQuote;
          else if (ch == '\'')
            cState = State::InSingleQuote;
          break;
      }
    }
  }
  
  std::string getResult()
  {
    return m_Result;
  }
  
  std::vector<std::string> getErrorlist()
  {
    return m_ErrorList;
  }
  
  void printErrors()
  {
    //for (auto i: m_ErrorList)
      //std::cout << i << std::endl;
  }

private:
	enum class State : char
	{
		SlashOC,
		SingleLineComment,
		NotAComment,
    
    InDoubleQuote,
    InSingleQuote,
    NoQuote,
	};
  
  bool onlySpace(const std::string &str)
  {
    return std::all_of(str.begin(),str.end(),isspace);
  }
  
  //file:line missing closing brackets
  void closingError()
  {
    char errStr[256];
    std::snprintf(errStr, 0, "%s missing bracket/paranthesis\n", m_FileName);
    std::string s(errStr);
    m_ErrorList.push_back(s);
  }
  
  void quoteError(size_t line)
  {
    char errStr[256];
    std::snprintf(errStr, 0, "%s:%u missing quote\n", m_FileName, line);
    std::string s(errStr);
    m_ErrorList.push_back(s);
  }
  
  std::string m_Result{};
  std::string &m_Data;
  char *m_FileName;
  std::vector<std::string> m_ErrorList{};
};

#endif