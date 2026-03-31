# Pull the pre-built CI image — avoids rebuilding Emscripten/AdaptiveCpp locally.
# USER_UID is injected by docker-compose from .devcontainer/.env (written by initializeCommand)
# so the container user's UID matches the host user's, keeping file ownership sane.
FROM ghcr.io/fair-acc/gr4-build-container:latest

ARG USER_UID=1000
USER root
RUN if [ "$(id -u user)" != "${USER_UID}" ]; then \
        usermod --non-unique -u "${USER_UID}" user \
     && chown -R user /home/user; \
    fi
USER user
RUN mkdir -p /home/user/.cache/ccache
ENTRYPOINT []
