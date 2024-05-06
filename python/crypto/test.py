import base64
import hashlib

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes

def generate_hash(data):
    # 创建一个sha256哈希对象
    sha256_hash = hashlib.sha256()
    # 对数据进行编码，因为hashlib只能处理字节类型的数据
    sha256_hash.update(data.encode('utf-8'))
    # 生成十六进制格式的哈希摘要
    hex_digest = sha256_hash.hexdigest()
    return hex_digest

def decrypt_and_verify(ciphertext, signature):
    decoded_data=base64.b64decode(ciphertext.encode('utf-8'))
    with open("../source/private_key.pem", "rb") as key_file:
        private_key = serialization.load_pem_private_key(
            key_file.read(),
            password=None,
            backend=default_backend()
        )
    original_message = private_key.decrypt(
        decoded_data,
        padding.OAEP(
            mgf=padding.MGF1(algorithm=hashes.SHA256()),
            algorithm=hashes.SHA256(),
            label=None
        )
    )
    decode = original_message.decode()
    hash_str = generate_hash(decode)
    return decode, hash_str == signature


if __name__ == '__main__':
    # # 加载私钥
    # with open("../source/private_key.pem", "rb") as key_file:
    #     private_key = serialization.load_pem_private_key(
    #         key_file.read(),
    #         password=None,
    #         backend=default_backend()
    #     )
    #
    # # 加载公钥
    # with open("../source/public_key.pem", "rb") as key_file:
    #     public_key = serialization.load_pem_public_key(
    #         key_file.read(),
    #         backend=default_backend()
    #     )
    # # 待加密的字符串
    # message = "d5e80a2f-de3e-4b6f-951b-0250e455f329"
    #
    # # 使用公钥加密
    # encrypted = public_key.encrypt(
    #     message.encode(),
    #     padding.OAEP(
    #         mgf=padding.MGF1(algorithm=hashes.SHA256()),
    #         algorithm=hashes.SHA256(),
    #         label=None
    #     )
    # )
    #
    # encoded_data = base64.b64encode(encrypted).decode('utf-8')
    #
    # print(f"加密后的消息: {encoded_data}")
    # # 使用私钥解密
    # original_message = private_key.decrypt(
    #     encrypted,
    #     padding.OAEP(
    #         mgf=padding.MGF1(algorithm=hashes.SHA256()),
    #         algorithm=hashes.SHA256(),
    #         label=None
    #     )
    # )
    #
    # print(f"原始消息: {message}")
    # print(f"解密后的消息: {original_message.decode()}")

    message = f"KU5A95c/DXWNzCksn9AdTGvkdnZ6K74+7ELJ4gh2pwbDj7KnzJhEK7QVbxDBdjJB8stRCxLmjQreGA9QN4Paitis64yDwlUxF4s8usZVBny9bvDbAdIV453pzsl9MtkDYffj35AhazRYdLamXM5rKC2nGcvzEqFhkeJYKzyvX8v/SYMyTjr9ge1updBSBzDMkRrOSOuJNUvcZzyEDGpraz8CnuRlv7Q4xMJbDbFGUeYYF7N8JP/uuTcoESFyHXx3lpVHKM2TsB4CEnP9nZUWYyuANo8x+BHfXDX2RQuoOHt+nSUOoKivWMebGJLCMN1cTd9LOyv9LnHsZNdacBidEg=="
    a, b = decrypt_and_verify(message, "60579a9e7fe6916e5e896d02a19b8456b4487c9f7f2bba81135d74a8a11f647e");
    print(a)
    print(b)

