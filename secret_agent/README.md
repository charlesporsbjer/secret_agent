# OpenSSL Configuration File for Creating a Root Certificate Authority (CA)

This configuration file is used to create and manage a Root Certificate Authority (CA) with OpenSSL. The CA is responsible for signing and issuing certificates that establish secure connections and validate the identity of servers, clients, or other entities.

## How It Works

1. **Root CA Creation**: The Root CA is created by generating a private key and a self-signed certificate. This certificate serves as the trusted root for all certificates issued by this CA.
2. **Configuration File Purpose**: A configuration file like this one ensures consistent settings for key generation, certificate signing requests (CSRs), and certificate issuance.
3. **Policies and Extensions**: Policies and extensions defined here enforce strict validation of certificate requests and provide additional information like key usage and constraints.
4. **Trust Inheritance**: Certificates issued by the CA inherit trust from the root certificate. Any entity with the root certificate installed can validate the authenticity of certificates issued by the CA.
5. **CA Infrastructure Management**: The CA infrastructure includes tracking issued certificates, revocation lists, and managing private keys securely.

## Configuration Sections

### `[ ca ]`
Specifies the default CA settings. The `default_ca` parameter points to the configuration block that contains the detailed CA settings. This allows multiple CA profiles to be defined in one file.

```ini
[ ca ]
default_ca = CA_default
```

### `[ CA_default ]`
Directory and file locations for the CA infrastructure.

```ini
[ CA_default ]
dir             = .                    # Base directory for the CA.
certs           = $dir/certs           # Directory for storing issued certificates.
crl_dir         = $dir/crl             # Directory for storing certificate revocation lists.
new_certs_dir   = $dir/newcerts        # Directory for newly issued certificates.
database        = $dir/index.txt       # Database file for tracking issued certificates.
serial          = $dir/serial          # File to maintain the next serial number for certificate issuance.
RANDFILE        = $dir/.rand           # Random seed file.
private_key     = $dir/ca.key          # Private key file for the CA.
certificate     = $dir/ca.crt          # Root certificate file for the CA.

default_md      = sha256               # Default message digest algorithm.
policy          = policy_strict        # Specifies the policy to use when issuing certificates.
```

### `[ policy_strict ]`
Rules for verifying the distinguished name fields in certificate requests.

```ini
[ policy_strict ]
countryName             = optional        # Must match the CA's country name.
stateOrProvinceName     = optional        # Must match the CA's state or province name.
organizationName        = optional        # Must match the CA's organization name.
organizationalUnitName  = optional        # Organizational unit is not required.
commonName              = supplied        # Common name must be provided.
emailAddress            = optional        # Email addresses are optional.
```

### `[ req ]`
Configuration for certificate signing requests (CSRs).

```ini
[ req ]
default_bits        = 2048             # Key size in bits.
default_md          = sha256           # Message digest for signing.
prompt              = no               # Disable interactive prompts.
default_keyfile     = ca.key           # Default private key file.
distinguished_name  = req_distinguished_name
```

### `[ req_distinguished_name ]`
Distinguished name fields for the CSR.

```ini
[ req_distinguished_name ]
C   = SE                              # Country name.
ST  = Stockholm                       # State or province name.
O   = STI                             # Organization name.
CN  = root.johan.se                   # Common name.
```

### `[ v3_ca ]`
Extensions for the CA certificate.

```ini
[ v3_ca ]
subjectKeyIdentifier   = hash         # Identifies the subject's public key.
authorityKeyIdentifier = keyid:always,issuer # Identifies the CA.
basicConstraints       = critical,CA:true # Specifies this as a CA certificate.
keyUsage               = critical,keyCertSign,cRLSign # Key usages allowed.
```

## Commands to Create the Root CA

1. **Generate the private key**:
   ```bash
   openssl genrsa -out ca.key 2048
   ```

2. **Create the CSR**:
   ```bash
   openssl req -config ca.cnf -new -key ca.key -out ca.csr
   ```

3. **Create the root certificate**:
   ```bash
   openssl x509 -req -days 365 -extensions v3_ca -signkey ca.key -in ca.csr -out ca.crt
   ```

## Instructions for Clients and Servers

1. **Create configuration files for client and server CSRs**:
   - For example, `client.cnf` and `server.cnf`.

2. **Generate private keys for client and server**:
   ```bash
   openssl genrsa -out client.key 2048
   openssl genrsa -out server.key 2048
   ```

3. **Create the CSRs**:
   ```bash
   openssl req -new -key client.key -out client.csr -config client.cnf
   openssl req -new -key server.key -out server.csr -config server.cnf
   ```

4. **Sign the CSRs with the CA's certificate and private key**:
   ```bash
   openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365 -sha256
   openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
   ```

5. **Distribute the signed certificates**:
   - Provide `client.crt` and `server.crt` to the respective entities.
