#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "ESP32_test"                         
 
const char WIFI_SSID[] = "NETGEAR94";               
const char WIFI_PASSWORD[] = "freshspider244";           
const char AWS_IOT_ENDPOINT[] = "a3ia2jrprhb7jg-ats.iot.us-east-1.amazonaws.com";       
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAOs4NvURztmiSNQUPSHBHD+S5O9GMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMjA5MjAxMjU1
MjFaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDnlDlENjagNpWm3fVv
3aOyO9xIUjTiDqUFd6Z109nSDfibpdOeOfnVXJ07nwVUkuv6eLjOXFcOsWq+YCF3
B8uftF6lGzcLQulPzdo7zPsk0vTKQyVjQ4YfTYLaZ8JXRLTnRLBZ5Rc7zbw3Dt2n
sAIaafHk+M6aGeVmR3+FbbaN3rhg2tHVgKqmf4t4YCPcVwr9cqU004dKoK+542UZ
ZWdQeuaQHAsw7G1fqFtJYegv/XrCTR6FiU30BkZUMDUY09lpPMW2q6Ufs7CxgcVx
qrcv3Y1w9yFFEHB4bEDRnwpTipN3JGUDzGx3JeB+SJjgf+Yom4FMshGX+4KSiNV+
DdRHAgMBAAGjYDBeMB8GA1UdIwQYMBaAFHWYkXtoKA9oK6q7SaPJzo1+AslQMB0G
A1UdDgQWBBSYN+MI1KyvXfVjvdMTkk+PnuZtoTAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAuZR+qlOe2XAFUS8Tog13FvM+
HxGuNt4dElNzt5ySam/aBaEp/lVt6dhH48F9F8oe2qnviJAyGpTS0dgZUfkxZol7
LDllT8WjFic+cRZ3LGt91Q7KqKZ9ZoCVuPKOsXTypZnY9AVVemZ/JQxr6v94rMqP
nSh+vsX6iPcMxEQykN0F3slkg++Q+K1N3leBvHkr9X5OjGGjG1c0nw65H08a7Dns
8fJjgwzmhPHmFkku7ZIhChmwz+tq9SjvfuTpQLRfiM/xqIbgNMQspXjUfZe5Huc9
l+IzLPG1zeS9AS4xL22LQJgg7RE+wPBa3jsbCjq5UxU/F9LmmT6VrRC7TFPVeg==
-----END CERTIFICATE-----
)KEY";
 
// Device Private Key                                            
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA55Q5RDY2oDaVpt31b92jsjvcSFI04g6lBXemddPZ0g34m6XT
njn51VydO58FVJLr+ni4zlxXDrFqvmAhdwfLn7RepRs3C0LpT83aO8z7JNL0ykMl
Y0OGH02C2mfCV0S050SwWeUXO828Nw7dp7ACGmnx5PjOmhnlZkd/hW22jd64YNrR
1YCqpn+LeGAj3FcK/XKlNNOHSqCvueNlGWVnUHrmkBwLMOxtX6hbSWHoL/16wk0e
hYlN9AZGVDA1GNPZaTzFtqulH7OwsYHFcaq3L92NcPchRRBweGxA0Z8KU4qTdyRl
A8xsdyXgfkiY4H/mKJuBTLIRl/uCkojVfg3URwIDAQABAoIBAA/pYFHug6xYc8nm
YFHSYcgCUu1pMJcDuJT6lqZr6wCC0z0mlLa/dwoQ9gC9uWkJO0uXdeM/9v9cc2An
TMVX9MbBZutjRlHwxAUmuML5H5x1HjqziGMcpZ+K/N2y2WiToPZvAsr6cbE7Pp5t
2A8Iiex0sRkxe0idVOTFLt7lHfD7ZiwTWMicaNPAV2hDCfXPk18y2oDgVaHE8vxK
QWAEbbGJyht8ZNyJ9mfjE8hA/MK9bx3Ic3m3SKGgzuecKOMyGCm5AlVuyQ7dC4MA
Wx2Q/n+1OS41H3M0VbOj8PAJ6Kp7rAPAh5+7/3ueMgvw7FjS5f4DezO6OiSRev+C
3W8TzCECgYEA9DRBf45oVU7K4XRFEkNvOoEr3mFBmfepi3WyIYnwhza7FaT3jS+S
tsbvcCZXzgKEc2OQJwZRf7IfjLHL5YIURt54PcB/9I+zgC3IYV71bvt2+0YFMO5d
+pVgTL1gltJyH2bAQs1s3tpau6rad7JM87+CoXC9kljmQAXCGu7B21ECgYEA8sPZ
ki49fS+dvoFEKM0Qc9TH2GikfQmYNuZf5pfwQiuhQBp4jhkROz0NE6ceDl49Zngs
1bfMZPlyuATMF68jcHEePR7eg9c6qvgMgBrjcPvZh67D6LO3sjULzo7+dCoJlkOj
yQ4WX6RQ7uxjtaH9kwUDM7PZyNK/GTg/JGtpIBcCgYAZqZAkfmuaY2BQlu5XP/zl
UgOxJ9lD+9G2VRT2D0/WdbrtSFu3W4N4b6ljyLO7c2pF0fWpFDHmUGqeV2TM5oGI
/QH3w03u/lbnpmgDS35RPl2/hBkxjisUFTR8e/41e0IA6MRVgM7rEFkulYLFx1fs
2Xg5zOYvBeaUXcaLHAIFMQKBgQCrsvle+nVi5RqccrDSarcINaZi2R/9j5uouKd7
YFr9cf5KaG57kh8t1rA5Bimr39ttbfHH9ocJo6QPotT2q1hVXqwUbpDSjpdf9LBi
qqHSd04TOPg2zWVPFVWUcm5lmpXNQWnF+cC5RG45jde3zyqZoSHyn3GZJ/Ug0TdX
euCPYQKBgGZIuPDdjbnzgDdGQLJL+LXGF+h7ONwIILjWx151nDaQoNLUJVuY08Kv
IGE4l26u4FjfSu0n0s1MjQIvdsGk0eR1V7A9MvYtkPbzKYZg/wVFVRXeLeJTvHdX
C8VerU2OprKWhk45fiCJlsWMoU99J9Tvghuc6OGjP+Ly1iV8sy67
-----END RSA PRIVATE KEY-----
)KEY";