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

For cross compile

```
make CROSS_COMPILE=<path/to/your/toolchian>
```

## Usage

```
sntpd [-h] [-r] [-m main_ntp_host] [-s side_ntp_host]

Options:
  -h       Show summary of command line options and exit
  -m       Main ntp server host (default: cn.pool.ntp.org)
  -s       Side ntp server host (default: pool.ntp.org)
  -t       Request interval (second, default: 3600)
  
```
