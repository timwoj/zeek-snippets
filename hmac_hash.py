import hmac

REQUEST_PATH='/broker'
TIMESTAMP=1614103772
BUILD_HASH='1f2c8d3f6753ad679d2023952d019f284e55f05bfa20ce6b73a02a95909cd486'
HMAC_KEY=b'0d99485ae504bebc60cbdf8dd3a740c3706da2b91b354e78b4fec21cc94e7ab4'

hmac_msg = '{:s}-{:d}-{:s}\n'.format(REQUEST_PATH, TIMESTAMP, BUILD_HASH)
digest = hmac.new(HMAC_KEY, hmac_msg.encode('utf-8'), 'sha256').hexdigest()
print(digest)
