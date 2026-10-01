import { useState } from 'react';
import { Monitor, Key, Terminal, CheckCircle, ArrowRight, ExternalLink } from 'lucide-react';
import { invoke } from '@tauri-apps/api/core';
import { useAppStore } from '../store/app-store';

type Step = 'welcome' | 'apikey' | 'steamlogin' | 'done';

const STEPS: Step[] = ['welcome', 'apikey', 'steamlogin', 'done'];

interface Props {
  onFinish: () => void;
}

export default function Onboarding({ onFinish }: Props) {
  const [step, setStep] = useState<Step>('welcome');
  const [apiKey, setApiKey] = useState('');
  const [username, setUsername] = useState('');
  const [loginOpened, setLoginOpened] = useState(false);
  const [loginError, setLoginError] = useState('');
  const { saveSettings, settings } = useAppStore();

  const stepIndex = STEPS.indexOf(step);

  const handleOpenTerminal = async () => {
    setLoginError('');
    try {
      await invoke('open_steamcmd_login', { username });
      setLoginOpened(true);
    } catch (e: unknown) {
      setLoginError(e instanceof Error ? e.message : String(e));
    }
  };

  const handleFinish = async () => {
    await saveSettings({
      ...settings,
      steam_api_key: apiKey || settings.steam_api_key,
      steam_username: username || settings.steam_username,
    });
    onFinish();
  };

  return (
    <div style={{
      position: 'fixed',
      inset: 0,
      background: 'var(--bg-base)',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      justifyContent: 'center',
      zIndex: 1000,
    }}>
      {/* Progress dots */}
      <div style={{ display: 'flex', gap: 8, marginBottom: 40 }}>
        {STEPS.map((s, i) => (
          <div key={s} style={{
            width: i <= stepIndex ? 24 : 8,
            height: 8,
            borderRadius: 4,
            background: i <= stepIndex ? 'var(--accent)' : 'var(--border)',
            transition: 'all 0.3s',
          }} />
        ))}
      </div>

      <div style={{
        width: '100%',
        maxWidth: 480,
        padding: '0 24px',
      }}>
        {step === 'welcome' && <WelcomeStep onNext={() => setStep('apikey')} />}
        {step === 'apikey' && (
          <ApiKeyStep
            value={apiKey}
            onChange={setApiKey}
            onNext={() => setStep('steamlogin')}
            onSkip={() => setStep('steamlogin')}
          />
        )}
        {step === 'steamlogin' && (
          <SteamLoginStep
            username={username}
            onUsernameChange={setUsername}
            onOpenTerminal={handleOpenTerminal}
            loginOpened={loginOpened}
            error={loginError}
            onNext={() => setStep('done')}
            onSkip={() => setStep('done')}
          />
        )}
        {step === 'done' && <DoneStep onFinish={handleFinish} />}
      </div>
    </div>
  );
}

function WelcomeStep({ onNext }: { onNext: () => void }) {
  return (
    <div style={cardStyle}>
      <div style={{ textAlign: 'center', marginBottom: 32 }}>
        <div style={{
          width: 64,
          height: 64,
          background: 'linear-gradient(135deg, #4d9eff, #a855f7)',
          borderRadius: 16,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          margin: '0 auto 20px',
        }}>
          <Monitor size={28} color="#fff" />
        </div>
        <h1 style={{ fontSize: 22, fontWeight: 700, marginBottom: 10, color: 'var(--text-primary)' }}>
          Wallpaper Engine Linux
        </h1>
        <p style={{ fontSize: 13, color: 'var(--text-secondary)', lineHeight: 1.6 }}>
          Browse Steam Workshop, download wallpapers, and set them on your Hyprland displays.
        </p>
      </div>

      <div style={{ display: 'flex', flexDirection: 'column', gap: 10, marginBottom: 32 }}>
        {[
          ['Steam Web API Key', 'Free — lets you browse the workshop'],
          ['Steam account login', 'Required to download workshop items'],
          ['awww + mpvpaper', 'Already installed on your system'],
        ].map(([title, desc]) => (
          <div key={title} style={{ display: 'flex', gap: 10, alignItems: 'flex-start' }}>
            <CheckCircle size={15} color="var(--accent)" style={{ flexShrink: 0, marginTop: 1 }} />
            <div>
              <div style={{ fontSize: 12, fontWeight: 600, color: 'var(--text-primary)' }}>{title}</div>
              <div style={{ fontSize: 11, color: 'var(--text-muted)' }}>{desc}</div>
            </div>
          </div>
        ))}
      </div>

      <NavButtons onNext={onNext} nextLabel="Get Started" />
    </div>
  );
}

function ApiKeyStep({
  value, onChange, onNext, onSkip,
}: {
  value: string;
  onChange: (v: string) => void;
  onNext: () => void;
  onSkip: () => void;
}) {
  return (
    <div style={cardStyle}>
      <StepHeader icon={<Key size={20} />} title="Steam Web API Key" />

      <p style={{ fontSize: 12, color: 'var(--text-secondary)', marginBottom: 20, lineHeight: 1.6 }}>
        Needed to browse and search the Steam Workshop. Free to get — takes 30 seconds.
      </p>

      <div style={{ marginBottom: 16 }}>
        <a
          href="https://steamcommunity.com/dev/apikey"
          target="_blank"
          rel="noreferrer"
          style={{
            display: 'inline-flex',
            alignItems: 'center',
            gap: 6,
            fontSize: 12,
            color: 'var(--accent)',
            textDecoration: 'none',
            marginBottom: 12,
          }}
        >
          <ExternalLink size={12} />
          steamcommunity.com/dev/apikey
        </a>
        <input
          value={value}
          onChange={(e) => onChange(e.target.value)}
          placeholder="Paste API key here…"
          type="password"
          style={inputStyle}
          autoFocus
        />
      </div>

      <div style={{ fontSize: 11, color: 'var(--text-muted)', marginBottom: 24, padding: '8px 12px', background: 'var(--bg-card)', borderRadius: 5 }}>
        Domain name field on Steam's page can be anything (e.g. <code>localhost</code>).
      </div>

      <NavButtons
        onNext={value.trim() ? onNext : undefined}
        onSkip={onSkip}
        nextLabel="Next"
        skipLabel="Skip for now"
      />
    </div>
  );
}

function SteamLoginStep({
  username, onUsernameChange, onOpenTerminal, loginOpened, error, onNext, onSkip,
}: {
  username: string;
  onUsernameChange: (v: string) => void;
  onOpenTerminal: () => void;
  loginOpened: boolean;
  error: string;
  onNext: () => void;
  onSkip: () => void;
}) {
  return (
    <div style={cardStyle}>
      <StepHeader icon={<Terminal size={20} />} title="Steam Login" />

      <p style={{ fontSize: 12, color: 'var(--text-secondary)', marginBottom: 20, lineHeight: 1.6 }}>
        steamcmd needs to authenticate once to cache credentials. After this, downloads work silently in the background.
      </p>

      <input
        value={username}
        onChange={(e) => onUsernameChange(e.target.value)}
        placeholder="Steam username"
        style={{ ...inputStyle, marginBottom: 12 }}
        autoFocus
      />

      <button
        onClick={onOpenTerminal}
        disabled={!username.trim()}
        style={{
          width: '100%',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          gap: 8,
          padding: '10px 16px',
          background: !username.trim() ? 'var(--bg-card)' : 'var(--bg-card-hover)',
          border: '1px solid var(--border)',
          borderRadius: 6,
          color: !username.trim() ? 'var(--text-muted)' : 'var(--text-primary)',
          fontSize: 13,
          fontWeight: 500,
          cursor: !username.trim() ? 'not-allowed' : 'pointer',
          marginBottom: 8,
          transition: 'all 0.15s',
        }}
      >
        <Terminal size={14} />
        Open terminal — steamcmd login
      </button>

      {loginOpened && (
        <div style={{ fontSize: 11, color: 'var(--success)', padding: '6px 10px', background: 'rgba(76,175,80,0.1)', borderRadius: 4, marginBottom: 8 }}>
          Terminal opened. Log in, then click Next.
        </div>
      )}

      {error && (
        <div style={{ fontSize: 11, color: 'var(--error)', padding: '6px 10px', background: 'rgba(244,67,54,0.1)', borderRadius: 4, marginBottom: 8 }}>
          {error}
        </div>
      )}

      <div style={{ fontSize: 11, color: 'var(--text-muted)', marginTop: 4 }}>
        Enter your password and Steam Guard code when prompted. The window closes automatically when done.
      </div>

      <div style={{ marginTop: 24 }}>
        <NavButtons onNext={onNext} onSkip={onSkip} nextLabel="Next" skipLabel="Skip for now" />
      </div>
    </div>
  );
}

function DoneStep({ onFinish }: { onFinish: () => void }) {
  return (
    <div style={{ ...cardStyle, textAlign: 'center' }}>
      <div style={{
        width: 64,
        height: 64,
        background: 'rgba(76,175,80,0.15)',
        border: '2px solid var(--success)',
        borderRadius: '50%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        margin: '0 auto 20px',
      }}>
        <CheckCircle size={28} color="var(--success)" />
      </div>

      <h2 style={{ fontSize: 20, fontWeight: 700, marginBottom: 10, color: 'var(--text-primary)' }}>
        All set!
      </h2>
      <p style={{ fontSize: 13, color: 'var(--text-secondary)', marginBottom: 32, lineHeight: 1.6 }}>
        You can change these settings anytime from the Settings panel.
      </p>

      <button
        onClick={onFinish}
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          gap: 8,
          width: '100%',
          padding: '12px 20px',
          background: 'var(--accent)',
          border: 'none',
          borderRadius: 6,
          color: '#fff',
          fontSize: 14,
          fontWeight: 700,
          cursor: 'pointer',
        }}
      >
        Browse Workshop
        <ArrowRight size={16} />
      </button>
    </div>
  );
}

function StepHeader({ icon, title }: { icon: React.ReactNode; title: string }) {
  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: 10, marginBottom: 20 }}>
      <div style={{
        width: 36,
        height: 36,
        background: 'rgba(77,158,255,0.12)',
        border: '1px solid rgba(77,158,255,0.2)',
        borderRadius: 8,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        color: 'var(--accent)',
      }}>
        {icon}
      </div>
      <h2 style={{ fontSize: 18, fontWeight: 700, color: 'var(--text-primary)' }}>{title}</h2>
    </div>
  );
}

function NavButtons({
  onNext, onSkip, nextLabel = 'Next', skipLabel,
}: {
  onNext?: () => void;
  onSkip?: () => void;
  nextLabel?: string;
  skipLabel?: string;
}) {
  return (
    <div style={{ display: 'flex', gap: 10, justifyContent: 'flex-end' }}>
      {onSkip && (
        <button onClick={onSkip} style={{
          padding: '9px 16px',
          background: 'transparent',
          border: '1px solid var(--border)',
          borderRadius: 6,
          color: 'var(--text-secondary)',
          fontSize: 13,
          cursor: 'pointer',
        }}>
          {skipLabel ?? 'Skip'}
        </button>
      )}
      {onNext && (
        <button
          onClick={onNext}
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 6,
            padding: '9px 18px',
            background: 'var(--accent)',
            border: 'none',
            borderRadius: 6,
            color: '#fff',
            fontSize: 13,
            fontWeight: 600,
            cursor: 'pointer',
          }}
        >
          {nextLabel}
          <ArrowRight size={14} />
        </button>
      )}
    </div>
  );
}

const cardStyle: React.CSSProperties = {
  background: 'var(--bg-surface)',
  border: '1px solid var(--border)',
  borderRadius: 12,
  padding: 28,
};

const inputStyle: React.CSSProperties = {
  width: '100%',
  padding: '9px 12px',
  background: 'var(--bg-card)',
  border: '1px solid var(--border)',
  borderRadius: 6,
  color: 'var(--text-primary)',
  fontSize: 13,
  display: 'block',
};
