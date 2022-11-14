#pragma once

#include <functional>

namespace SoftRenderer
{
	/**
	* The ID of a listener (Registered callback).
	* This value is needed to remove a listener from an event
	*/
	using ListenerID = uint64_t;

	/**
	* A simple event that contains a set of function callbacks. These functions will be called on invoke
	*/
	template<typename... ArgTypes>
	class Event
	{
	public:
		/**
		* Simple shortcut for a generic function without return value
		*/
		using Callback = std::function<void(ArgTypes...)>;

		/**
		* Add a function callback to this event
		* Also return the ID of the new listener (You should store the returned ID if you want to remove the listener later)
		* @param p_call
		*/
		ListenerID addListener(Callback callback);

		/**
		* Add a function callback to this event
		* Also return the ID of the new listener (You should store the returned ID if you want to remove the listener later)
		* @param p_call
		*/
		ListenerID operator+=(Callback callback);

		/**
		* Remove a function callback to this event using a Listener (Created when calling AddListener)
		* @param p_listener
		*/
		bool removeListener(ListenerID listenerID);

		/**
		* Remove a function callback to this event using a Listener (Created when calling AddListener)
		* @param p_listener
		*/
		bool operator-=(ListenerID listenerID);

		/**
		* Remove every listeners to this event
		*/
		void removeAllListeners();

		/**
		* Return the number of callback registered
		*/
		uint64_t getListenerCount();

		/**
		* Call every callbacks attached to this event
		* @param p_args (Variadic)
		*/
		void invoke(ArgTypes... p_args);

	private:
		std::unordered_map<ListenerID, Callback>	mCallbacks;
		ListenerID									mAvailableListenerID = 0;
	};

    template<class... ArgTypes>
    ListenerID Event<ArgTypes...>::addListener(Callback callback)
    {
        ListenerID listenerID = mAvailableListenerID++;
        mCallbacks.emplace(listenerID, callback);
        return listenerID;
    }

    template<class... ArgTypes>
    ListenerID Event<ArgTypes...>::operator+=(Callback callback)
    {
        return addListener(callback);
    }

    template<class... ArgTypes>
    bool Event<ArgTypes...>::removeListener(ListenerID listenerID)
    {
        return mCallbacks.erase(listenerID) != 0;
    }

    template<class... ArgTypes>
    bool Event<ArgTypes...>::operator-=(ListenerID listenerID)
    {
        return removeListener(listenerID);
    }

    template<class... ArgTypes>
    void Event<ArgTypes...>::removeAllListeners()
    {
        mCallbacks.clear();
    }

    template<class... ArgTypes>
    uint64_t Event<ArgTypes...>::getListenerCount()
    {
        return mCallbacks.size();
    }

    template<class... ArgTypes>
    void Event<ArgTypes...>::invoke(ArgTypes... args)
    {
        for (auto const& [key, value] : mCallbacks)
            value(args...);
    }
}