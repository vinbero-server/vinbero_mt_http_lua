FROM vinbero/vinbero
MAINTAINER Byeonggon Lee (gonny952@gmail.com)

EXPOSE 80
COPY config.json /srv/config.json
COPY config2.json /srv/config2.json
COPY config.yaml /srv/config.yaml
COPY app.lua /srv/app.lua

RUN apk update && apk add http-parser-dev lua5.3-dev

RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_tcp /usr/src/vinbero_tcp
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_mt /usr/src/vinbero_mt
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_strm_mt_epoll /usr/src/vinbero_strm_mt_epoll
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_mt_epoll_tls /usr/src/vinbero_mt_epoll_tls
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_mt_epoll_http /usr/src/vinbero_mt_epoll_http
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_iplogger /usr/src/vinbero_iplogger
RUN git clone --recurse-submodules -j8 https://github.com/vinbero/vinbero_mt_http_lua /usr/src/vinbero_mt_http_lua

RUN mkdir /usr/src/vinbero_tcp/build; cd /usr/src/vinbero_tcp/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_mt/build; cd /usr/src/vinbero_mt/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_strm_mt_epoll/build; cd /usr/src/vinbero_strm_mt_epoll/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_mt_epoll_tls/build; cd /usr/src/vinbero_mt_epoll_tls/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_mt_epoll_http/build; cd /usr/src/vinbero_mt_epoll_http/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_iplogger/build; cd /usr/src/vinbero_iplogger/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir /usr/src/vinbero_mt_http_lua/build; cd /usr/src/vinbero_mt_http_lua/build; cmake -DCMAKE_C_FLAGS="-I /usr/include/lua5.3 -L /usr/lib/lua5.3" -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install

CMD ["/usr/bin/vinbero", "-c", "/srv/config.json", "-f", "63"]
