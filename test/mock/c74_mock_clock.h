/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

using namespace std::chrono_literals;

namespace c74 {
namespace mock {
	
	using function = std::function<void(void)>;
	using generic_callback = void(*)(void*); // TODO: convert to std::function?

	using clock_type = std::chrono::time_point<std::chrono::steady_clock>;
	using lock = std::lock_guard<std::mutex>;
	using timepoint = std::chrono::steady_clock::time_point;
	using duration_ms = std::chrono::milliseconds;
	
	class clock {
		
		class event {
			timepoint	m_onset;
//			function	m_meth;
			max::method	m_meth;
			void*		m_baton;

		public:
			event(timepoint onset, max::method meth, void* baton)
			: m_onset(onset)
			, m_meth(meth)
			, m_baton(baton)
			{}
			
			
			friend bool operator< (const event& lhs, const event& rhs) {
				return lhs.m_onset < rhs.m_onset;
			}
		
			friend bool operator< (const event& lhs, const timepoint rhs) {
				return lhs.m_onset < rhs;
			}

			friend bool operator > (const event& lhs, const timepoint rhs) {
				return lhs.m_onset > rhs;
			}

			void operator ()() const {
				m_meth(m_baton);
			}
			
			auto onset() const {
				return m_onset;
			}
		};
		
		
	public:
		clock(max::method a_callback, void* a_baton)
		: meth(a_callback)
		, baton(a_baton)
		{}
		
		~clock() {
			t.join();
		}
		
		// TODO: mark class as not copyable?
		
		auto time(clock_type onset) {
			return std::chrono::duration_cast<std::chrono::milliseconds>(onset - started_at).count();
		}
		
		auto now() {
			return std::chrono::high_resolution_clock::now();
		}
		
		void tick() {
			while (true) {
#ifdef WIN_VERSION
				// On Windows an exception will be thrown when we try to lock the mutex wihout some sort of time passage
				// (either a console post or an explicit short delay)
				// std::cout << c74::mock::clock::tick()" << std::endl;
				std::this_thread::sleep_for(1ms);
#endif
				std::vector<event>				events_to_run;
				std::unique_lock<std::mutex>	lock(mutex);	// locks mutex immediately, guarantees unlock on dtor
    
				// update list of events
				for (auto &e : new_events)
					events.insert(e);
				new_events.clear();
				
				// service the list of events
				{
					lock.unlock();	// only protecting new_events, and we don't want to call into user code below from within the lock!
					for (auto& e : events) {
						if (e > now()) {
							//std::cout << time(now()) << " (all remaining events in the future)" << std::endl;
							break;
						}
						//std::cout << time(now()) << " (event to run)" << std::endl;
						events_to_run.push_back(e);
						e();
					}
					
					// purge events that have run
					for (auto& e : events_to_run)
						events.erase(e);

					lock.lock(); // need to re-lock before accessing new_events and cv
				}
				
				// events themselves may have added new (pending) events
				for (auto &e : new_events)
					events.insert(e);
				new_events.clear();

				// decide when to run again
				if (!events.empty()) {
					auto& e = *events.begin();
					auto next = e.onset();
					//std::cout << time(now()) << " SLEEP UNTIL " << time(next) << std::endl;
					cv.wait_until(lock, next);
				}
				else {
					//std::cout << time(now()) << " SLEEP FOREVER!" << std::endl;
					cv.wait_until(lock, now()+1h);
				}
			}
		}
		
		void add(const std::chrono::milliseconds delay/*, function meth*/) {
			auto			onset = now() + delay;
			clock::event	event {onset, meth, baton};

			//std::cout << time(now()) << " NEW EVENT: " << std::chrono::duration_cast<std::chrono::milliseconds>(onset - started_at).count() << "   " << baton << std::endl;
			
			// a previous version of this code incorrectly tried to deal with deadlock by checking to see if the tick thread was the one adding a new event.
			// but if the tick thread was adding an event simultaneously to another thread then the content of new_events would be undefined, so we need to lock it for all writing.
			// the deadlock was occurring because the tick() method was calling into user code while locked, and the user code (in the metro case) was adding new events.
			
			lock l(mutex);
			new_events.push_back(event);
			cv.notify_one();
		}
		
		/** Convenience wrapper for adding events with a callback to a class member method without having to manually setup std::bind.
			@param delay	time offset from now() at which to execute
			@param meth		pointer to member method to be called
			@param target	pointer to this
		 */
//		template<class MethType, class T>
//		void add(const std::chrono::milliseconds delay, MethType meth, T* target) {
//			add(delay, std::bind(meth, target));
//		}
		
		
		const std::thread::id threadid() const {
			return t.get_id();
		}
		
	private:
		clock_type				started_at = { std::chrono::steady_clock::now() };
		std::thread				t { &clock::tick, this };
		std::mutex				mutex;
		std::multiset<event>	events;	// using multiset instead of vector because we want to keep it sorted, performance impacts?
		std::condition_variable	cv;
		std::vector<event>		new_events;
		
		max::method				meth;
		void*					baton;
	};
	
}} //namespace c74::mock


namespace c74 {
namespace max {
	
	using t_clock = mock::clock;

	MOCK_EXPORT t_clock* clock_new(void* obj, method fn) {
		
		// TODO: we're going to run into trouble freeing this with object_free...
		
		return new mock::clock(fn, obj);
	}

	
	MOCK_EXPORT void clock_unset(t_clock* self) {
		// TODO: implement
	}
	
	
	MOCK_EXPORT void clock_fdelay(t_clock* self, double duration_in_ms) {
		self->add(std::chrono::milliseconds((int)duration_in_ms));
	}
	
		
}} // namespace c74::max
