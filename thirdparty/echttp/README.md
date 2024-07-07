# ECHTTP
ECHTTP (pronounced somewhat like "easy HTTP" I'd say) is a simple single-file HTTP/HTTPS client library written in C.

This implementation is roughly based on [http.h by Mattias Gustavsson](https://github.com/mattiasgustavsson/libs/blob/main/http.h), but has been refactored and improved, including **TLS support**, **more than just GET and POST requests**, and an **easier API**.

It uses [TLSe](https://github.com/eduardsui/tlse) for TLS (previously known as SSL) support.

## License
This project is licensed under the [MIT License](LICENSE).