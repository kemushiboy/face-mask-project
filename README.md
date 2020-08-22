# face-mask-project


### 前準備
- touchDesigner  2020 25380 をインストール

- homebrewをインストール

- xcode, command line toolsをインストール

- gitをインストール

- Cloneしてくる
git clone https://github.com/kemushiboy/face-mask-project.git

ディレクトリ名をsicfに変更

### IRIS Trackingnの設定

- install Bazel.
`$ brew install bazel`
Run ‘bazel version’ to check version of bazel

- Install OpenCV and FFmpeg.
`$ brew install opencv@3`

- There is a known issue caused by the glog dependency. Uninstall glog.
`$ brew uninstall  --ignore-dependencies glog`

- ビルドする
`./build.sh`

### TouchDesignerとpythonとの連携設定
- pipをインストール
`Sudo easy_install pip`

- pyenvをインストール
`brew install  pyenv`

- pipenvをインストール
`brew install pipenv`

- pipenvでパッケージの保存先が直下ディレクトリになるようにする
`echo ‘export PIPENV_VENV_IN_PROJECT=true’ >> ~/.bash_profile  pipenv shell` 

- protobufをインストール
`pipenv shell`
`pip3 install protobuf`

- wrapper_iris_tracking_pb2.pyのシンボリックリンクをパッケージフォルダに作成
`ln -s  /Users/tatsuya/sicf/wrapper_iris_tracking_pb2.py  /Users/tatsuya/sicf/.venv/lib/python3.7/site-packages`

- TDでパッケージの場所を
`/Users/tatsuya/sicf/.venv/lib/python3.7/site-packages`
に指定する
