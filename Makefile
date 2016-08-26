#CXXFLAGS =	-O3 -Wall -fmessage-length=0 -std=c++11
CXXFLAGS = -fPIC -g -O3 -Wall -fmessage-length=0 -std=c++14
CFLAGS = -Wno-address -Wno-char-subscripts # -Wno-sign-compare

SWIFT=$(wildcard src/*.cpp)
LIBSWIFTHEADERS=$(wildcard src/*.h)
TEST=test.cpp
CXXSOURCES=$(SWIFT)
TESTSOURCES=$(TEST)
#CSOURCES=httpxx/http_parser.c

CXXOBJS=$(CXXSOURCES:%.cpp=%.o)
TESTOBJS=$(TESTSOURCES:%.cpp=%.o)
#COBJS=$(CSOURCES:%.c=%.o)

#build dir
BUILDDIR=build

#Libraries
LDFLAGS = -shared

LIBS = -lPocoUtil -lPocoXML -lPocoNet -lPocoFoundation -lpthread


TARGET =	SwiftSDK
LIBSWIFT = $(BUILDDIR)/libSwift.a

#CXX=clang++
all: $(LIBSWIFT) $(TARGET)

$(LIBSWIFT) : $(CXXOBJS)
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/include/Swift
	ar rcs $@ $(CXXOBJS)
	cp -rf $(LIBSWIFTHEADERS) $(BUILDDIR)/include/Swift

$(TARGET):	$(CXXOBJS) $(COBJS) $(TESTOBJS)
#	$(CXX) -o $(TARGET) $(CXXOBJS) $(LIBS) $(COBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(CXXOBJS) $(TESTOBJS) $(LIBS)

install:
	mkdir -p $(DESTDIR)/include
	mkdir -p $(DESTDIR)/lib
	cp -r $(BUILDDIR)/include/Swift $(DESTDIR)/include
	cp $(LIBSWIFT) $(DESTDIR)/lib/libSwift.a

#$(COBJS): %.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(CXXOBJS) $(TARGET) $(TESTOBJS) $(LIBSWIFT) $(wildcard build/*) $(BUILDDIR)

# distro for package building (oneof: wily, fedora-23-x86_64)
DISTRIBUTION        ?= none

PKG_REVISION    ?= $(shell git describe --tags --always)
PKG_VERSION	?= $(shell git describe --tags --always | tr - .)
PKG_BUILD       ?= 1
PKG_ID           = swift-sdk-cpp-$(PKG_VERSION)

.PHONY: rpm deb

check_distribution:
ifeq ($(DISTRIBUTION), none)
	@echo "Please provide package distribution. Oneof: 'wily', 'fedora-23-x86_64'"
	@exit 1
else
	@echo "Building package for distribution $(DISTRIBUTION)"
endif

%/$(PKG_ID).tar.gz:
	mkdir -p $*
	rm -rf $*/$(PKG_ID)
	git archive --format=tar --prefix=$(PKG_ID)/ $(PKG_REVISION)| (cd $* && tar -xf -)
	find $*/$(PKG_ID) -depth -name ".git" -exec rm -rf {} \;
	tar -C $* -czf $*/$(PKG_ID).tar.gz $(PKG_ID)

deb:
	mkdir -p package/$(DISTRIBUTION)/source package/$(DISTRIBUTION)/binary-amd64
	./bamboos/make.py -e DISTRIBUTION=$(DISTRIBUTION) --privileged --group sbuild -i onedata/deb_builder package_deb

package_deb: check_distribution deb/$(PKG_ID).tar.gz
	rm -rf deb/packages && mkdir -p deb/packages
	mv -f deb/$(PKG_ID).tar.gz deb/swift-sdk-cpp_$(PKG_VERSION).orig.tar.gz
	cp -R pkg_config/debian deb/$(PKG_ID)/
	sed -i "s/swift-sdk-cpp (.*) .*;/swift-sdk-cpp ($(PKG_VERSION)-$(PKG_BUILD)) wily;/g" deb/$(PKG_ID)/debian/changelog
	sed -i "s/Build from .*/Build from $(PKG_VERSION)/g" deb/$(PKG_ID)/debian/changelog

	cd deb/$(PKG_ID) && sg sbuild -c "sbuild -sd $(DISTRIBUTION) -j4"

	mv deb/*$(PKG_VERSION).orig.tar.gz package/$(DISTRIBUTION)/source
	mv deb/*$(PKG_VERSION)-$(PKG_BUILD).dsc package/$(DISTRIBUTION)/source
	mv deb/*$(PKG_VERSION)-$(PKG_BUILD).diff.gz package/$(DISTRIBUTION)/source || \
	mv deb/*$(PKG_VERSION)-$(PKG_BUILD).debian.tar.xz package/$(DISTRIBUTION)/source
	mv deb/*$(PKG_VERSION)-$(PKG_BUILD)_amd64.changes package/$(DISTRIBUTION)/source
	mv deb/*$(PKG_VERSION)-$(PKG_BUILD)*.deb package/$(DISTRIBUTION)/binary-amd64

rpm:
	mkdir -p package/$(DISTRIBUTION)/SRPMS package/$(DISTRIBUTION)/x86_64
	./bamboos/make.py -e DISTRIBUTION=$(DISTRIBUTION) --privileged --group mock -i onedata/rpm_builder package_rpm

package_rpm: check_distribution rpm/$(PKG_ID).tar.gz
	rm -rf rpm/packages && mkdir -p rpm/packages
	mv -f rpm/$(PKG_ID).tar.gz rpm/$(PKG_ID).orig.tar.gz
	cp pkg_config/swift-sdk-cpp.spec rpm/swift-sdk-cpp.spec
	sed -i "s/{{version}}/$(PKG_VERSION)/g" rpm/swift-sdk-cpp.spec
	sed -i "s/{{build}}/$(PKG_BUILD)/g" rpm/swift-sdk-cpp.spec

	mock --root $(DISTRIBUTION) --buildsrpm --spec rpm/swift-sdk-cpp.spec --resultdir=rpm/packages \
		--sources rpm/$(PKG_ID).orig.tar.gz
	mock --root $(DISTRIBUTION) --resultdir=rpm/packages --rebuild rpm/packages/$(PKG_ID)*.src.rpm

	mv rpm/packages/*.src.rpm package/$(DISTRIBUTION)/SRPMS
	mv rpm/packages/*.x86_64.rpm package/$(DISTRIBUTION)/x86_64

package.tar.gz:
	tar -chzf package.tar.gz package