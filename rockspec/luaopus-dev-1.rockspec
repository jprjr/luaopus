package = "luaopus"
version = "dev-1"

source = {
  url = "git+https://github.com/jprjr/luaopus.git"
}

description = {
  summary = "Lua bindings to libopus",
  homepage = "https://github.com/jprjr/luaopus",
  license = "MIT"
}

build = {
  type = "builtin",
  modules = {
    ["luaopus.version"] = "src/luaopus/version.lua",
    ["luaopus"] = {
      libdirs = "$(OPUS_LIBDIR)",
      incdirs = "$(OPUS_INCDIR)",
      libraries = "opus",
      sources = {
        "csrc/luaopus.c",
        "csrc/luaopus_decoder.c",
        "csrc/luaopus_defines.c",
        "csrc/luaopus_encoder.c",
        "csrc/luaopus_internal.c",
      },
    },
  }
}

dependencies = {
  "lua >= 5.1",
}

external_dependencies = {
  OPUS = {
    header = 'opus/opus.h',
    library = 'opus',
  },
}


