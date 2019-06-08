#include "connection.h"
#include <functional>
#include <iostream>

int Connection::connect()
{
	// initilize database
	return 0;
}

std::shared_ptr<Connection::QueryResult> Connection::query(const char * txt)
{
	return std::shared_ptr<QueryResult>(new QueryResult(0));
}
