#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string>

using std::string;

class CException
{
 public:
  CException(string p_strMessage) { m_strMessage = p_strMessage; }
  virtual ~CException() {};

  string& GetMessage() { return m_strMessage; }

 private:
  string m_strMessage;
};

#endif
