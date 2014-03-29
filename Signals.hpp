/*
* 
* Copyright (C) Patryk Jaworski <regalis@regalis.com.pl>
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
*/

#ifndef __REGALIS_SIGNALS__
#define __REGALIS_SIGNALS__

#include <functional>
#include <list>
#include <memory>

namespace regalis {

template <class... Args>
class Signal;

template <class... Args>
class Slot;


/** Class template representing Signal of specified type.
 *
 * Each signal can store multiple Slots of appropriate type,
 * all of them will be called when particular event occurs.
 *
 * User can emit specified signal by calling emit() or
 * by using operator()()
 *
 * Both Signal and Slot are type-safe.
 *
 * @param Args signal types
 */
template <class... Args>
class Signal {
	public:
		/** Type of Slot which can be used with this Signal */
		typedef Slot<Args...> slot_t;

		/** Connect a Slot to this Signal
		 * 
		 * @param slot pointer to a Slot
		 */
		void connect(std::shared_ptr<slot_t> slot) {
			slots.push_back(slot);
			slot->_signal = this;
		}

		/** Disconnect specified Slot from the Signal
		 *
		 * After successfull disconnection, specified Slot
		 * is removed from the list of slots inside the Signal and
		 * slot's internal pointer to signal is cleared.
		 *
		 * @param slot pointer to a Slot you want to disconnect
		 * @return true after successfull disconnection, false
		 * otherwise
		 */
		bool disconnect(std::shared_ptr<slot_t> slot) {
			for (auto it = slots.begin(); it != slots.end(); ++it) {
				if ((*it).get() == slot.get()) {
					slots.erase(it);
					slot->_signal = nullptr;
					return true;
				}
			}
			return false;
		}
		
		/** Emit the Signal by calling all of previously
		 * connected slots.
		 *
		 * @param args list of args which will be passed to
		 * all of connected slots
		 */
		void emit(Args... args) {
			for (auto &x : slots) {
				(*x)(args...);
			}
		}

		/** Emit the Signal by calling all of previously
		 * connected slots.
		 *
		 * @param args list of args which will be passed to
		 * all of connected slots
		 */
		void operator()(Args... args) {
			emit(args...);
		}
		
	protected:
		std::list<std::shared_ptr<slot_t>> slots;
};

/** Class representing single Slot of specified type
 *
 * Slot can be connected to Signal of the same type
 * and is responsible for storing target function.
 *
 * Note! Single Slot instance represents **single connection**,
 * not single terget function. If you need to connect the same
 * function/method to two different Signals, you need two Slot
 * instances.
 *
 * There are two ways for creating Slot instances, first
 * by using one of Slot::create() static methods, second by using 
 * function connect().
 *
 * Note! In both ways you will receive a smart pointer to newly
 * created instances.
 *
 * In many cases saving pointer to Slot is not required. It is only
 * needed when you are going to disconnect already connected Slot.
 *
 * @param Args Slot arguments
 */
template <class... Args>
class Slot {
	public:
		/** Type of Signal which can be used with this slot */
		typedef Signal<Args...> signal_t;

		// Prevent from copying
		Slot(const Slot<Args...>&) = delete;
		Slot<Args...> &operator=(const Slot<Args...>&) = delete;


		template <class T>
		static std::shared_ptr<Slot<Args...>> create(T &t, void(T:: *f)(Args...)) {
			std::shared_ptr<Slot<Args...>> slot(new Slot<Args...>(t, f));
			return slot;
		}
		
		static std::shared_ptr<Slot<Args...>> create(std::function<void(Args...)> f) {
			std::shared_ptr<Slot<Args...>> slot(new Slot<Args...>(f));
			return slot;
		}

		/** Check if slot is already connected
		 * @return pointer to signal or NULL if slot is not
		 * connected
		 */
		signal_t *isConnected() {
			return _signal;
		}

		/** Disconnect Slot from the Signal
		 * @return true after successfull disconnection, false otherwise
		 */
		bool disconnect() {
			if (isConnected() != nullptr)
				return _signal->disconnect(this);
			return false;
		}

		/** Make slot inactive
		 *
		 * Inactive slots won't be called when emit occurs
		 */
		void makeInactive() {
			_active = false;
		}

		/** Make slot active */
		void makeActive() {
			_active = true;
		}

		/** Check if slot is active
		 * @return true if slot is active, false otherwise
		 */
		bool isActive() {
			return _active;
		}

		virtual ~Slot() {}

	protected:
		std::function<void(Args...)> _slot_function;
		signal_t *_signal = nullptr;
		bool _active = true;

		/** Create a Slot pointing to member function of type T */
		template <class T>
		Slot(T &t, void(T:: *f)(Args...)) {
			_slot_function = [&t, f](Args... args){(t.*f)(args...);};
		}

		/** Create a Slot pointing to function */
		Slot(std::function<void(Args...)> f) {
			_slot_function = std::move(f);
		}

		/** Call slot function */
		virtual void operator()(Args... args) {
			if (_active)
				_slot_function(args...);
		}

	friend class Signal<Args...>;
};

/** Connect specified Signal with class member of type T
 *
 * Function creates a Slot which store pointer to member function and
 * connects this Slot to specified Signal.
 *
 * @param Args type of Signal we want to connect
 * @arg sig Signal we want to connect to
 * @arg t object on which we call member function
 * @arg f pointer to a member function
 * @return smart pointer to connected Slot
 */
template <class T, class... Args>
std::shared_ptr<Slot<Args...>> connect(Signal<Args...> &sig, T &t, void(T:: *f)(Args...)) {
	auto slot = Slot<Args...>::create(t, f);
	sig.connect(slot);
	return slot;
}

/** Connect specified Signal with function
 *
 * Function creates a Slot which store pointer to function and
 * connects this Slot to specified Signal.
 *
 * @param Args type of Signal we want to connect
 * @arg sig Signal we want to connect to
 * @arg f pointer to a function
 * @return smart pointer to connected Slot
 */
template <class... Args>
std::shared_ptr<Slot<Args...>> connect(Signal<Args...> &sig, void(*f)(Args...)) {
	auto slot = Slot<Args...>::create(f);
	sig.connect(slot);
	return slot;
}

}

#endif
