
# Stage 1: Build dependencies and compile the C code
FROM gcc:11 AS builder
WORKDIR /app
COPY . .
RUN gcc -o myapp src/main.c

# Stage 2: Create the minimal runtime image
FROM debian:bullseye-slim
WORKDIR /app
RUN mkdir -p /bin /dev /proc
COPY --from=builder /app/myapp .
CMD ["./myapp"]