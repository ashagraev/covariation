covariations-test: main.cpp
	g++ -std=c++11 -o $@ $<

errors.txt: covariations-test
	./covariations-test > errors.txt

clean:
	rm -f covariations-test errors.txt
