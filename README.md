# sntpd

A tiny sntp protocol implement for [rfc4330](https://www.rfc-editor.org/info/rfc4330)

## Compliance

Just simply type the make command in the project root directory

```
make
```

For verbose command output during compile

```
make V=1
```

For compile with debug symbol

```
make D=1
```

For cross compile

```
make CROSS_COMPILE=<path/to/your/toolchian>
```

## Usage

Quick start

```
sntpd -d
```

For usage prompt

```
sntpd -h
```

Dry run, will only send sntp request but not upgrade system time

```
sntp -n
```
