NAME := arduino-timer-cpp17
VERSION := $(shell git describe --tags --always --dirty)

$(NAME)-$(VERSION).zip:
	git archive HEAD --prefix=$(NAME)/ --format=zip -o $@

tag:
	git tag $(VERSION)
