import socket
import time
import wrapper_hand_tracking_pb2

M_SIZE = 1024
# 
host = '127.0.0.1'
port = 8080

locaddr = (host, port)

# ①ソケットを作成する
sock = socket.socket(socket.AF_INET, type=socket.SOCK_DGRAM)
print('create socket')

# ②自ホストで使用するIPアドレスとポート番号を指定
sock.bind(locaddr)


def ListHandTracking(wrapperHandTracking):

    #NormalizedLandmarkList landmarks = 1;
    #optional NormalizedRect rect = 2;
    #optional DetectionList detection = 3;

    #for landmark in wrapperHandTracking.landmarks:
    print ("Landmarks: ",wrapperHandTracking.landmarks)


while True:
    try :
        # ③Clientからのmessageの受付開始
        print('Waiting message')
        message, cli_addr = sock.recvfrom(M_SIZE)
        #message = message.decode(encoding='utf-8')

        #*todo ここでmessageをprotobufのWrapperHandTracking形式でdecodeする
        wrapper_hand_tracking = wrapper_hand_tracking_pb2.WrapperHandTracking()
        wrapper_hand_tracking.ParseFromString(message)
        ListHandTracking(wrapper_hand_tracking)

        #print(f'Received message is [{message}]')

        # Clientが受信待ちになるまで待つため
        #time.sleep(1)

        # ④Clientへ受信完了messageを送信
        #print('Send response to Client')
        #sock.sendto('Success to receive message'.encode(encoding='utf-8'), cli_addr)

    except KeyboardInterrupt:
        print ('\n . . .\n')
        sock.close()
        break
