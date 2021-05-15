all: check format lint build

build: render-ci-pipeline build-c

miner: run
run: build-c
    ./src/miner

cc: build-c
build-c: buildc
buildc:
    ./src/build.sh

render-ci-pipeline:
    ./scripts/render-ci-pipeline.sh

fmt: format

format: format-dhall prettier format-shfmt format-c

lint: lint-dhall shellcheck

check: check-dhall

prettier:
    yarn run prettier

fmt-c: format-c
fmtc: format-c
format-c:
    ./scripts/c-format.sh

format-dhall:
    ./scripts/dhall-format.sh

check-dhall:
    ./scripts/dhall-check.sh

lint-dhall:
    ./scripts/dhall-lint.sh

shellcheck:
    ./scripts/shellcheck.sh

format-shfmt:
    shfmt -w .

install:
    just install-asdf
    just install-yarn

install-yarn:
    yarn

install-asdf:
    asdf install
