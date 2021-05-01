#pragma once

namespace Arclight {

class NonCopyable {
protected:
	NonCopyable() = default;
	NonCopyable(NonCopyable&&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;

	virtual ~NonCopyable() = default;

private:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

} //namespace Arclight