TSAN = g++ -std=c++17 -fsanitize=thread
ASAN = g++ -std=c++17 -fsanitize=address

queue_SOURCES = queue_test.cc bounded_mpmc.h non_intrusive_mpsc.h

queue_test: queue_test_TSAN queue_test_ASAN
	./queue_test_TSAN && ./queue_test_ASAN

queue_test_TSAN: $(queue_SOURCES)
	$(TSAN) $^ -o $@

queue_test_ASAN: $(queue_SOURCES)
	$(ASAN) $^ -o $@

clean:
	rm -f *_TSAN *_ASAN
