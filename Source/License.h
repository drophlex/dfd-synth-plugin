// Dark Funk Durcal Synt: the 3-day trial and the full-version code. (c) Polaris Sound Lab, Mérida, Yucatán, México.
//
// The code is a signature of the buyer's e-mail: SHA-256("DFDS2|" + e-mail, trimmed and lower-case) raised to the
// private exponent modulo N (a 576-bit RSA key); the plugin only holds N and e, so it can check a code but not make
// one. The code is written in Crockford base-32, "DFD2-" then groups of 8 (dashes and spaces are ignored).
// The trial starts the first time the plugin opens on a computer and is stored twice (the app-data settings and a
// hidden file in the home folder), each with a check value, so deleting or editing one does not restart it; a clock
// turned back by more than a day ends it.
#pragma once
#include <JuceHeader.h>

namespace dfdlic {

static constexpr const char* kModulusHex =
    "d34ac302d88d8298a6c46a95101eb787dc5c7a69eb1b0112fc1a4d9eae33dc872006b141afefbaa4f20322e45f9908d122d129900bcb15557448bc5e9a2aebea34efaa87b3290761";
static constexpr int kExponent = 65537;
static constexpr juce::int64 kTrialMs = (juce::int64)3 * 24 * 60 * 60 * 1000;
static constexpr const char* kBuyUrl = "https://drophlex.github.io/dark-funk-durcal-synt-fm1/#comprar";
static constexpr const char* kInstagramUrl = "https://ig.me/m/polaris.sound.lab";
static constexpr const char* kEmail = "polarissoundlab@gmail.com";

inline juce::String normaliseEmail(const juce::String& e) { return e.trim().toLowerCase(); }

inline juce::BigInteger fromBytes(const juce::uint8* p, size_t n) {
    juce::BigInteger v;
    for (size_t i = 0; i < n; ++i) { v <<= 8; v += (int)p[i]; }
    return v;
}

inline juce::BigInteger emailHash(const juce::String& email) {
    auto text = ("DFDS2|" + normaliseEmail(email)).toStdString();
    juce::SHA256 h(text.data(), text.size());
    auto raw = h.getRawData();
    return fromBytes((const juce::uint8*)raw.getData(), raw.getSize());
}

// Crockford base-32 (0-9 A-Z without I L O U; I and L read as 1, O as 0)
inline bool decodeCode(const juce::String& code, juce::BigInteger& out) {
    static const char* A = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
    auto s = code.toUpperCase().removeCharacters(" -_\t\r\n");
    if (s.startsWith("DFD2")) s = s.substring(4);
    if (s.length() < 20) return false;
    juce::BigInteger v;
    for (auto c : s) {
        if (c == 'I' || c == 'L') c = '1';
        if (c == 'O') c = '0';
        auto p = std::strchr(A, (int)c);
        if (p == nullptr || c == 0) return false;
        v <<= 5; v += (int)(p - A);
    }
    out = v;
    return true;
}

inline bool checkCode(const juce::String& email, const juce::String& code) {
    if (!normaliseEmail(email).containsChar('@')) return false;
    juce::BigInteger s, n, e(kExponent);
    if (!decodeCode(code, s)) return false;
    n.parseString(kModulusHex, 16);
    if (s.isZero() || s.compare(n) >= 0) return false;
    s.exponentModulo(e, n);
    return s == emailHash(email);
}

// ---------------------------------------------------------------- the trial and the activation, on disk
struct State {
    bool licensed = false;
    juce::String email;
    juce::int64 trialStart = 0;
    bool clockBack = false;
    juce::int64 msLeft() const { return trialStart + kTrialMs - juce::Time::currentTimeMillis(); }
    bool trialOver() const { return clockBack || msLeft() <= 0; }
    bool canPlay() const { return licensed || !trialOver(); }
    int daysLeft() const { return juce::jmax(0, (int)((msLeft() + 86399999) / 86400000)); }
};

inline juce::String tag(juce::int64 v) {
    auto s = ("ps-dfd-" + juce::String(v) + "-mx").toStdString();
    return juce::SHA256(s.data(), s.size()).toHexString().substring(0, 16);
}

inline juce::File settingsFile() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Polaris Sound Lab").getChildFile("DarkFunkDurcalSynt.settings");
}
inline juce::File markerFile() {
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile(".polaris_dfds");
}

inline juce::int64 readMarker(const juce::File& f, juce::int64& seen) {
    auto lines = juce::StringArray::fromLines(f.loadFileAsString());
    if (lines.size() < 3) return 0;
    auto t0 = lines[0].getLargeIntValue();
    if (lines[1] != tag(t0)) return 0;
    seen = lines[2].getLargeIntValue();
    return t0;
}

inline State load() {
    State st;
    const auto now = juce::Time::currentTimeMillis();
    juce::PropertiesFile::Options o;
    juce::PropertiesFile pf(settingsFile(), o);
    juce::int64 t0 = 0, seen = 0, seen2 = 0;
    if (auto a = pf.getValue("t0").getLargeIntValue(); a > 0 && pf.getValue("k") == tag(a)) { t0 = a; seen = pf.getValue("seen").getLargeIntValue(); }
    auto t1 = readMarker(markerFile(), seen2);
    if (t1 > 0 && (t0 == 0 || t1 < t0)) t0 = t1;
    seen = juce::jmax(seen, seen2);
    if (t0 <= 0 || t0 > now + 86400000) t0 = now;               // the first time here (or a broken date): it starts now
    st.trialStart = t0;
    st.clockBack = seen > now + 86400000;                        // the clock went back by more than a day
    st.email = pf.getValue("email");
    st.licensed = checkCode(st.email, pf.getValue("code"));
    // write both back (the trial start never moves later)
    pf.setValue("t0", juce::String(t0)); pf.setValue("k", tag(t0)); pf.setValue("seen", juce::String(juce::jmax(seen, now)));
    pf.saveIfNeeded();
    markerFile().replaceWithText(juce::String(t0) + "\n" + tag(t0) + "\n" + juce::String(juce::jmax(seen, now)) + "\n");
    return st;
}

inline bool activate(const juce::String& email, const juce::String& code) {
    if (!checkCode(email, code)) return false;
    juce::PropertiesFile::Options o;
    juce::PropertiesFile pf(settingsFile(), o);
    pf.setValue("email", normaliseEmail(email));
    pf.setValue("code", code.trim());
    pf.saveIfNeeded();
    return true;
}

} // namespace dfdlic
