TSAN = g++ -std=c++17 -fsanitize=thread
ASAN = g++ -std=c++17 -fsanitize=address

list_SOURCES = list_test.cc skip_list.h linked_list.h

list_test: list_test_TSAN list_test_ASAN
	./list_test_TSAN
	./list_test_ASAN

list_test_TSAN: $(list_SOURCES)
	$(TSAN) $^ -o $@

list_test_ASAN: $(list_SOURCES)
	$(ASAN) $^ -o $@

queue_SOURCES = queue_test.cc bounded_mpmc.h non_intrusive_mpsc.h

queue_test: queue_test_TSAN queue_test_ASAN
	./queue_test_TSAN
	./queue_test_ASAN

queue_test_TSAN: $(queue_SOURCES)
	$(TSAN) $^ -o $@

queue_test_ASAN: $(queue_SOURCES)
	$(ASAN) $^ -o $@

clean:
	rm -f *_TSAN *_ASAN
