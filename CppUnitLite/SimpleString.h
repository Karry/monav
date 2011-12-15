#pragma once


class SimpleString
{
public:
	SimpleString();
	SimpleString(const char *value);
	SimpleString(const SimpleString& other);
	~SimpleString();

	char*			GetCString () const;
	unsigned int	GetLength() const;

	SimpleString&	operator=	(const SimpleString& other);
	void			operator+=	(const char* rhs);
	void			operator+=	(const SimpleString& rhs);

private:
	char* mBuffer;


private:	// Helpers and friend declarations.
	char* CreateEmptyCString(unsigned int size);


        friend bool	operator== (const SimpleString& left, const SimpleString& right);
};



SimpleString StringFrom (bool value);
SimpleString StringFrom (const char *value);
SimpleString StringFrom (short value);
SimpleString StringFrom (unsigned short value);
SimpleString StringFrom (int value);
SimpleString StringFrom (unsigned int value);
SimpleString StringFrom (long value);
SimpleString StringFrom (unsigned long value);
SimpleString StringFrom (double value);
SimpleString StringFrom (const SimpleString& other);
