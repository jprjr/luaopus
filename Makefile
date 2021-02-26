.PHONY: release github-release

VERSION = $(shell LUA_PATH="./src/?.lua" lua -e 'print(require("luaopus.version")._VERSION)')

github-release:
	source $(HOME)/.github-token && github-release release \
	  --user jprjr \
	  --repo luaopus \
	  --tag v$(VERSION)
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo luaopus \
	  --tag v$(VERSION) \
	  --name luaopus-$(VERSION).tar.gz \
	  --file dist/luaopus-$(VERSION).tar.gz
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo luaopus \
	  --tag v$(VERSION) \
	  --name luaopus-$(VERSION).tar.xz \
	  --file dist/luaopus-$(VERSION).tar.xz

release:
	rm -rf dist/luaopus-$(VERSION)
	rm -rf dist/luaopus-$(VERSION).tar.gz
	rm -rf dist/luaopus-$(VERSION).tar.xz
	mkdir -p dist/luaopus-$(VERSION)/csrc
	rsync -a csrc/ dist/luaopus-$(VERSION)/csrc/
	rsync -a src/ dist/luaopus-$(VERSION)/src/
	rsync -a CMakeLists.txt dist/luaopus-$(VERSION)/CMakeLists.txt
	rsync -a LICENSE dist/luaopus-$(VERSION)/LICENSE
	rsync -a README.md dist/luaopus-$(VERSION)/README.md
	sed 's/@VERSION@/$(VERSION)/g' < rockspec/luaopus-template.rockspec > dist/luaopus-$(VERSION)/luaopus-$(VERSION)-1.rockspec
	cd dist && tar -c -f luaopus-$(VERSION).tar luaopus-$(VERSION)
	cd dist && gzip -k luaopus-$(VERSION).tar
	cd dist && xz luaopus-$(VERSION).tar

