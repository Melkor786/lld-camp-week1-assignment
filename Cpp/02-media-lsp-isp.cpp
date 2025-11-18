// 02-media-lsp-isp-solved.cpp

#include <iostream>
#include <string>
using namespace std;

// ----------------- Small Focused Interfaces (ISP) -------------------

class IPlayable {
public:
    virtual ~IPlayable() = default;
    virtual void play(const string& source) = 0;
    virtual bool isPlaying() const = 0;
};

class IPausable {
public:
    virtual ~IPausable() = default;
    virtual void pause() = 0;
};

class IDownloadable {
public:
    virtual ~IDownloadable() = default;
    virtual void download(const string& url) = 0;
};

class ILiveStreamable {
public:
    virtual ~ILiveStreamable() = default;
    virtual void startStream(const string& url) = 0;
    virtual bool isLive() const = 0;
};

class IRecordable {
public:
    virtual ~IRecordable() = default;
    virtual void record(const string& dest) = 0;
};

// ----------------- Concrete Classes (LSP Safe) ---------------------

// Audio player only supports play, pause, download.
class AudioPlayer : public IPlayable, public IPausable, public IDownloadable {
    bool playing{false};

public:
    void play(const string& src) override {
        (void)src;
        playing = true;
    }

    void pause() override {
        playing = false;
    }

    void download(const string& url) override {
        (void)url; /* simulate download */
    }

    bool isPlaying() const override {
        return playing;
    }
};

// Camera stream player explicitly requires live stream before play.
// No hidden preconditions → LSP safe.
class LiveStreamPlayer : public IPlayable, public IPausable, 
                         public ILiveStreamable, public IRecordable 
{
    bool live{false};
    bool playing{false};

public:
    void startStream(const string& url) override {
        (void)url;
        live = true;
    }

    bool isLive() const override {
        return live;
    }

    void play(const string& src) override {
        (void)src;
        if (!live) 
            throw runtime_error("Must start live stream before play()!");

        playing = true;
    }

    void pause() override {
        playing = false;
    }

    void record(const string& dest) override {
        (void)dest; /* simulate record */
    }

    bool isPlaying() const override {
        return playing;
    }
};

// ----------------- Client Code ---------------------

int main() {
    AudioPlayer ap;
    ap.play("song.mp3");
    cout << "Audio playing: " << boolalpha << ap.isPlaying() << "\n";
    ap.pause();

    LiveStreamPlayer cam;
    cam.startStream("rtsp://camera");
    cam.play("rtsp://camera");
    cout << "Camera playing: " << boolalpha << cam.isPlaying() << "\n";

    return 0;
}
