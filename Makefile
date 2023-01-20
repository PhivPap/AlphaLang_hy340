all:
	cd AlphaC && $(MAKE) && cp alphac ../alphac
	cd AlphaVM && $(MAKE) && cp alpha ../alpha

clean:
	rm -f alphac alpha
	cd AlphaC && $(MAKE) clean
	cd AlphaVM && $(MAKE) clean
