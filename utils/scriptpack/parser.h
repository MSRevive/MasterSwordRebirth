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
  Parser(std::string data, char *file) : m_Data(data) {
    m_FileName = file;
    m_Result = m_Data;
  }
  
  //credits to https://codereview.stackexchange.com/a/215913
  void stripComments()
  {
    State cState = State::NotAComment;
    std::string res;
    
    for (size_t i = 0; i < m_Result.length(); i++)
    {
      const char ch = m_Result[i];
      switch (cState)
      {
        case State::SlashOC:
          if (ch == '/')
            cState = State::SingleLineComment;
          else
          {
            // ?????
            cState = State::NotAComment;
            res += ch;
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
    std::istringstream ss(m_Result);
    std::string line;
    size_t lineNum = 0;
    
    while (std::getline(ss, line)) {
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
              else if (pos == (line.length()))
              {
                cState = State::NoQuote;
                quoteError(lineNum, pos);
              }
              break;
            case State::InSingleQuote:
              if (ch == '\'')
                cState = State::NoQuote;
              else if (pos == (line.length()))
              {
                cState = State::NoQuote;
                quoteError(lineNum, pos);
              }
              break;
            case State::InPara:
              if (ch == ')')
                cState = State::NoQuote;
              else if (pos == (line.length()))
              {
                cState = State::NoQuote;
                addError("%s:%u.%u: expected closing parentheses", lineNum, pos);
              }
              break;
            case State::NoQuote:
              if (ch == '"')
                cState = State::InDoubleQuote;
              else if (ch == '\'')
                cState = State::InSingleQuote;
              else if (ch == '(')
                cState = State::InPara;
              break;
          }
        }
      }
    }
  }
  
  void checkBrackets()
  {
    std::istringstream ss(m_Result);
    std::string line;
    std::vector<std::pair<size_t, size_t>> openBrace{};
    size_t lineNum = 1;
    //std::cout << m_Result << std::endl;
    
    while (std::getline(ss, line, '\n'))
    {
      //std::cout << line << std::endl;
      for(int pos = 0; pos < line.length(); pos++)
      {
        switch(line[pos])
        {
          case '{':
            std::cout << "push " << lineNum << std::endl;
            openBrace.push_back(std::make_pair(lineNum, pos));
            break;
          case '}':
            std::cout << "pop " << lineNum << std::endl;
            if (openBrace.size() > 0) //don't try to pop empty vector, bad things happen...
              openBrace.pop_back();
            break;
        }
      }

      lineNum++;
    }
    
    for (std::pair<size_t, size_t> ob : openBrace)
      addError("%s:%u.%u: brace opened, but never closed", ob.first, ob.second);
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
    std::cout << std::endl;
  }
  
  void saveErrors()
  {
    std::ofstream o;
    o.open("./errors.txt", std::ios_base::app | std::ios_base::out);
    
    for (auto i: m_ErrorList)
      o << i << std::endl;
      
    o.close();
  }

  void saveResult(char *create)
  {
    std::ofstream o(create);
    o << m_Result;
    o.close();
  }

private:
  enum class State : char
  {
    SlashOC,
    SingleLineComment,
    NotAComment,
    
    InDoubleQuote,
    InSingleQuote,
    InPara,
    NoQuote,
  };
  
  bool onlySpace(const std::string &str)
  {
    return std::all_of(str.begin(), str.end(), isspace);
  }
  
  void addError(const char *fmt, size_t lineNum, size_t pos)
  {
    char eBuffer[256];
    snprintf(eBuffer, 256, fmt, m_FileName, lineNum, pos);
    std::string s(eBuffer);
    m_ErrorList.push_back(s);
  }
  
  void quoteError(size_t line, size_t pos)
  {
    addError("%s:%u.%u: unclosed quotation", line, pos);
  }
  
  std::string m_Result{};
  std::string m_Data;
  char *m_FileName;
  std::vector<std::string> m_ErrorList{};
};

#endif