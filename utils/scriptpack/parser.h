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
    m_Result = m_Data;
  }
  
  //credits to https://codereview.stackexchange.com/a/215913
  void stripComments()
	{
    State cState = State::NotAComment;
		const size_t size = m_Result.length();
    std::string res;
    
		for (size_t idx = 0; idx < size; ++idx)
		{
			const char ch = m_Result[idx];
			switch (cState)
			{
			case State::SlashOC:
				if (ch == '/')
					cState = State::SingleLineComment;
				else
				{
					// ?????
					cState = State::NotAComment;
					res += '/' + ch;
				}
				break;

			case State::SingleLineComment:
				if (ch == '\n')
        {
          cState = State::NotAComment;
          res += '\n';
        }
				break;
        
      case State::NotAComment:
				if (ch == '/')
					cState = State::SlashOC;
				else
					res += ch;
				break;
			}
		}
    
    m_Result = res;
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
  
  void checkQuotes()
  {
    State cState = State::NoQuote;
    std::stringstream ss(m_Result);
    size_t lineNum = 0;
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
      lineNum++;
      
      //don't process if line is empty
      if (!line.empty() && !onlySpace(line))
      {
        for (size_t pos = 0; pos < line.length(); pos++)
        {
          const char ch = line[pos];
          
          switch(cState)
          {
            case State::InDoubleQuote:
              if (ch == '"')
                cState = State::NoQuote;
              else if (pos == (line.length() - 1))
              {
                cState = State::NoQuote;
                quoteError(lineNum, pos);
              }
              break;
            case State::InSingleQuote:
              if (ch == '\'')
                cState = State::NoQuote;
              else if (pos == (line.length() - 1))
              {
                cState = State::NoQuote;
                quoteError(lineNum, pos);
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
    }
  }
  
  //credits to https://codereview.stackexchange.com/a/40518
  bool bracketsMatch() {
    std::stringstream ss(m_Result);
    std::stack<char> expectedDelimiters;
    size_t lineNum = 0;
    std::string line;
    
    while (std::getline(ss, line)) {
      lineNum++;
      size_t pos = 0;
      
      while (std::string::npos != (pos = line.find_first_of("(){}[]", pos))) {
        size_t colNum = pos + 1;
        switch (line[pos]) {
          case '(': 
            expectedDelimiters.push(')'); 
            break;
          case '{': 
            expectedDelimiters.push('}'); 
            break;
          case '[': 
            expectedDelimiters.push(']'); 
            break;

          case ']':
          case '}':
          case ')':
            if (expectedDelimiters.empty()) {
              addError("%s:%u.%u: mismatched brackets/parentheses", lineNum, colNum);
              return false;
            }
            if (line[pos] != expectedDelimiters.top()) {
              addError("%s:%u.%u: expected closing bracket/parentheses", lineNum, colNum);
              return false;
            }
            expectedDelimiters.pop();
            break;
        }
        
        pos = colNum;
      }
    }
    // // Should check for a possible input error here, but I didn't bother.
    // if (!expectedDelimiters.empty()) {
    // 
    //   return false;
    // }
    return true;
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
    std::cout << m_ErrorList.size() << " errors found" << std::endl;
    for (auto i: m_ErrorList)
      std::cout << i << std::endl;
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
  
  //to do: do generic variadic arguments
  //file:lineNum:linePos
  void addError(char *msg, size_t lineNum, size_t linePos = 0)
  {
    char eBuffer[256];
    std::snprintf(eBuffer, 256, msg, m_FileName, lineNum, linePos);
    std::string s(eBuffer);
    m_ErrorList.push_back(s);
  }
  
  void quoteError(size_t line, size_t pos)
  {
    addError("%s:%u:%u - missing quotation", line, pos);
  }
  
  std::string m_Result{};
  std::string &m_Data;
  char *m_FileName;
  std::vector<std::string> m_ErrorList{};
};

#endif