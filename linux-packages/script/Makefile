PACKAGE_NAME="trenchbroom"
RELEASE_VERSION="2.0"
ITERATION="1"
GITHUB_REPO="kduske/TrenchBroom"
GITHUB_BRANCH="develop"
BUILD_DIR=$(PACKAGE_NAME)/build
VENDOR="Kristian Duske <kristian.duske@gmail.com>"
MAINTAINER="Kristian Duske <kristian.duske@gmail.com>"
URL="http://kristianduske.com/trenchbroom/"
DESCRIPTION="TrenchBroom - A Modern Level Editor for Quake"

checkvars:
	@echo "Packagename:     "$(PACKAGE_NAME)
	@echo "Releaseversion:  "$(RELEASE_VERSION)
	@echo "Github Repo:     "$(GITHUB_REPO)
	@echo "Github Branch:   "$(GITHUB_BRANCH)
	@echo "Build Directory  "$(BUILD_DIR)
	@echo "Maintainer:      "$(MAINTAINER)

fetch:
	wget https://github.com/$(GITHUB_REPO)/archive/$(GITHUB_BRANCH).tar.gz
	mv $(GITHUB_BRANCH).tar.gz $(PACKAGE_NAME).tar.gz
	mkdir $(PACKAGE_NAME)
	tar --strip-components 1 --directory $(PACKAGE_NAME) -xf $(PACKAGE_NAME).tar.gz

gitfetch:
	rm -rf $(PACKAGE_NAME)
	git clone https://github.com/$(GITHUB_REPO).git $(PACKAGE_NAME)
	cd $(PACKAGE_NAME) && git checkout $(GITHUB_BRANCH)

build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)	&& cmake .. -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --target TrenchBroom

deb:
	fpm -s dir -t deb -n $(PACKAGE_NAME) -v $(RELEASE_VERSION) \
	    --vendor $(VENDOR) \
	    --maintainer $(MAINTAINER) \
	    --license "GPLv3" \
	    --url $(URL) \
	    --category "games" \
	    --description $(DESCRIPTION) \
	    --after-install "after-install.sh" \
	    --after-remove "after-remove.sh" \
	    --architecture $(shell dpkg --print-architecture) \
	    --iteration $(ITERATION) \
	    --depends "$(shell dpkg-shlibdeps -e $(BUILD_DIR)/TrenchBroom -O 2>/dev/null | cut -d'=' -f2-)" \
	    $(BUILD_DIR)/TrenchBroom=/opt/trenchbroom/ \
	    $(BUILD_DIR)/Resources=/opt/trenchbroom/ \
	    ./copyright=/opt/trenchbroom/ \
	    ./desktop/trenchbroom.desktop=/usr/share/applications/ \
	    ./desktop/icon48x48/trenchbroom.png=/usr/share/icons/hicolor/48x48/apps/ \
	    ./desktop/icon256x256/trenchbroom.png=/usr/share/icons/hicolor/256x256/apps/
	    

rpm:
	fpm -s dir -t rpm -n $(PACKAGE_NAME) -v $(RELEASE_VERSION) \
	    --vendor $(VENDOR) \
	    --maintainer $(MAINTAINER) \
	    --license "GPLv3" \
	    --url $(URL) \
	    --category "games" \
	    --description $(DESCRIPTION) \
	    --after-install "after-install.sh" \
	    --after-remove "after-remove.sh" \
	    --architecture $(shell dpkg --print-architecture) \
	    --iteration $(ITERATION) \
	    --depends "$(shell readelf -d $(BUILD_DIR)/TrenchBroom | grep Shared | awk '{print $$5}' | tr -d '[]' | sort | uniq | tr '\n' ' ' )" \
	    $(BUILD_DIR)/TrenchBroom=/opt/trenchbroom/ \
	    $(BUILD_DIR)/Resources=/opt/trenchbroom/ \
	    ./copyright=/opt/trenchbroom/
	    ./desktop/trenchbroom.desktop=/usr/share/applications/ \
	    ./desktop/icon48x48/trenchbroom.png=/usr/share/icons/hicolor/48x48/apps/ \
	    ./desktop/icon256x256/trenchbroom.png=/usr/share/icons/hicolor/256x256/apps/


tar:
	mkdir /trenchbroom-$(RELEASE_VERSION)
	cp -a $(BUILD_DIR)/TrenchBroom /trenchbroom-$(RELEASE_VERSION)
	cp -a $(BUILD_DIR)/Resources /trenchbroom-$(RELEASE_VERSION)
	cp -a ./copyright /trenchbroom-$(RELEASE_VERSION)
	cp -a ./desktop/icon256x256/trenchbroom.png /trenchbroom-$(RELEASE_VERSION)
	cp -a ./desktop/trenchbroom.desktop /trenchbroom-$(RELEASE_VERSION)
	tar -czf trenchbroom-$(RELEASE_VERSION)-$(ITERATION)-$(shell dpkg --print-architecture).tar.gz /trenchbroom-$(RELEASE_VERSION)

movepkg:
	mv *.deb *.rpm trenchbroom-$(RELEASE_VERSION)-$(ITERATION)-$(shell dpkg --print-architecture).tar.gz /pkg

