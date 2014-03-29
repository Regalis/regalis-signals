# C++11 Signals and Slots

Minimalistic implementation of signals and slots mechanism written in C++11 using some of new features including variadic templates, lambdas and smart pointers. **Note**: current version is not thread-safe.

# What it is and when to use it?

Signals and slots mechanism is powerfull and easy to use alternative to the callback technique. Regalis Signals can be used when you need to notify some objects or functions when particular event ocurs. Let's see an example below:

```C++
/* File: Counter.hpp */
#ifndef __COUNTER_HPP_
#define __COUNTER_HPP_

#include "Signals.hpp"

class Counter {
	public:
		void inc();
		void dec();
		void reset();
		
		regalis::Signal<int> incremented;
		regalis::Signal<int> decremented;
		regalis::Signal<> reseted;
	protected:
		int _value = 0;
};

#endif
```

In this example we have three Signals: **incremented** which will be used to notify every interested object/function after incrementation, **decremented** which will be used to notify about decrementation and **reseted** which will notify about reset action. Let's see how to implement this methods:

```C++
void Counter::inc() {
	incremented(++_value);
}
```

**Note**: It's the same as:

```C++
void Counter::inc() {
	++_value; // increment _value
	incremented.emit(_value); // notify all interested objects/functions about new value
}
```

Now let's see how to connect this signals to specific methods/functions using Slots:

```C++
/* File: CounterMonitor.hpp */
#ifndef __COUNTER_MONITOR_HPP_
#define __COUNTER_MONITOR_HPP_
#include "Counter.hpp"

class CounterMonitor {
	public:
		CounterMonitor(Counter *counter);
		void incremented(int);
		void decremented(int);
		void reseted();
};

#endif
```

Constructor **CounterMonitor(Counter\*)** whill be used to connect Counter's signals to CounterMonitor's methods:

```C++
CounterMonitor::CounterMonitor(Counter *counter) {
	regalis::connect(counter->incremented, this, CounterMonitor::incremented);
	regalis::connect(counter->decremented, this, CounterMonitor::decremented);
	regalis::connect(counter->reseted, this, CounterMonitor::reseted);
}
```

Now let's write *main()* function and try to use it:

```C++
/* File: main.cpp */
#include "Signals.hpp"
#include "Counter.hpp"
#include "CounterMonitor.hpp"

void local_function(int new_counter_value) {
	std::count << "Counter value changed: " << new_counter_value << std::endl;
}

int main() {
	Counter counter;
	CounterMonitor monitor(&counter);
	
	/** We can connect all of 3 signals to one function */
	regalis::connect(counter.incremented, local_function);
	regalis::connect(counter.decremented, local_function);
	regalis::connect(counter.reseted, local_function);

	counter.inc();
	counter.inc();
	counter.dec();
	counter.reset();

}
```

# How to start

Before you start using this library, make sure you ran:

	$ make test

**Note**: probably some compilers won't compile this code, make sure you are using the latest version of your compiler.

# Todo

* more examples
* explenation of internals
* more tests
* make it thread-safe

License
=======

Copyright (C) 2014 Patryk Jaworski \<regalis@regalis.com.pl\>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses.
