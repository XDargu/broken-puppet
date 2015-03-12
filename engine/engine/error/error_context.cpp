#include "mcv_platform.h"

// Global error context
std::vector<const char *> _error_context_name;
std::vector<const char *> _error_context_data;

CErrorContext::CErrorContext(const char *name, const char *data) {
	_error_context_name.push_back(name);
	_error_context_data.push_back(data);	
}
CErrorContext::~CErrorContext() {
	_error_context_name.pop_back();
	_error_context_data.pop_back();
}
