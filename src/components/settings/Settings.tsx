import { useState } from 'react';
import { Key, FolderOpen, Monitor, User, Volume2, Gauge, Download } from 'lucide-react';
import { invoke } from '@tauri-apps/api/core';
import { useAppStore } from '../../store/app-store';

export default function Settings() {
  const { settings, saveSettings, displays, toolsStatus, checkTools, showToast } = useAppStore();
  const [form, setForm] = useState({ ...settings });
  const [downloadingAssets, setDownloadingAssets] = useState(false);

  const handleDownloadAssets = async () => {
    if (!form.steam_username) {
      showToast('Set Steam username first', 'error');
      return;
    }
    setDownloadingAssets(true);
    try {
      await invoke('download_we_assets', { username: form.steam_username });
      showToast('Terminal opened — log in and wait for download to finish, then click Refresh Tools', 'info');
    } catch (e: unknown) {
      showToast(e instanceof Error ? e.message : String(e), 'error');
    } finally {
      setDownloadingAssets(false);
    }
  };

  const update = (key: keyof typeof form, value: string | boolean | number) => {
    setForm((f) => ({ ...f, [key]: value }));
  };

  const handleSave = () => saveSettings(form);

  return (
    <div style={{ flex: 1, overflowY: 'auto', padding: '20px 24px' }}>
      <h1 style={{ fontSize: 16, fontWeight: 700, marginBottom: 20, color: 'var(--text-primary)' }}>
        Settings
      </h1>

      <Section title="Steam Integration">
        <Field
          icon={<Key size={13} />}
          label="Steam Web API Key"
          hint={
            <span>
              Get free key at{' '}
              <span style={{ color: 'var(--accent)', cursor: 'pointer' }}>
                steamcommunity.com/dev/apikey
              </span>
            </span>
          }
        >
          <input
            type="password"
            value={form.steam_api_key}
            onChange={(e) => update('steam_api_key', e.target.value)}
            placeholder="Your Steam API key…"
            style={inputStyle}
          />
        </Field>

        <Field
          icon={<User size={13} />}
          label="Steam Username"
          hint="Used with steamcmd to download workshop items"
        >
          <input
            value={form.steam_username}
            onChange={(e) => update('steam_username', e.target.value)}
            placeholder="steam username (optional)"
            style={inputStyle}
          />
        </Field>

        <StatusRow
          label="steamcmd"
          ok={toolsStatus.steamcmd}
          okText="Installed"
          failText="Not found — install with: yay -S steamcmd"
        />
      </Section>

      <Section title="Wallpaper">
        <Field
          icon={<FolderOpen size={13} />}
          label="Download Directory"
          hint="Workshop items and local wallpapers stored here"
        >
          <input
            value={form.download_dir}
            onChange={(e) => update('download_dir', e.target.value)}
            placeholder="~/.local/share/wpe-linux/wallpapers"
            style={inputStyle}
          />
        </Field>

        <Field
          icon={<Monitor size={13} />}
          label="Default Display"
          hint="Display to target when applying wallpapers"
        >
          <select
            value={form.default_monitor}
            onChange={(e) => update('default_monitor', e.target.value)}
            style={{ ...inputStyle, cursor: 'pointer' }}
          >
            <option value="">Auto (focused display)</option>
            {displays.map((d) => (
              <option key={d.name} value={d.name}>
                {d.name} — {d.width}×{d.height}@{Math.round(d.refresh_rate)}Hz
              </option>
            ))}
          </select>
        </Field>

        <Field
          icon={<Volume2 size={13} />}
          label="Audio"
          hint="Enable audio for video wallpapers (mpvpaper)"
        >
          <label style={{ display: 'flex', alignItems: 'center', gap: 8, cursor: 'pointer' }}>
            <Toggle
              checked={form.audio_enabled}
              onChange={(v) => update('audio_enabled', v)}
            />
            <span style={{ fontSize: 12, color: 'var(--text-secondary)' }}>
              {form.audio_enabled ? 'Enabled' : 'Disabled'}
            </span>
          </label>
        </Field>

        <Field
          icon={<Gauge size={13} />}
          label="FPS Limit"
          hint="Limit wallpaper framerate to reduce CPU/GPU usage"
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
            <input
              type="range"
              min={5}
              max={60}
              step={5}
              value={form.fps_limit}
              onChange={(e) => update('fps_limit', Number(e.target.value))}
              style={{ flex: 1, accentColor: 'var(--accent)' }}
            />
            <span style={{ fontSize: 12, color: 'var(--text-secondary)', minWidth: 40 }}>
              {form.fps_limit} fps
            </span>
          </div>
        </Field>
      </Section>

      <Section title="Dependencies">
        <StatusRow label="awww (swww)" ok={toolsStatus.awww} okText="Installed — images & GIFs" failText="Not found — yay -S awww" />
        <StatusRow label="hyprpaper" ok={toolsStatus.hyprpaper} okText="Installed — static fallback" failText="Not found — yay -S hyprpaper" />
        <StatusRow label="mpvpaper" ok={toolsStatus.mpvpaper} okText="Installed — video wallpapers" failText="Not found — yay -S mpvpaper" />
        <StatusRow label="linux-wallpaperengine" ok={toolsStatus.linux_wallpaperengine} okText="Installed — scene wallpapers" failText="Not found — yay -S linux-wallpaperengine" />
        <StatusRow label="WE assets" ok={toolsStatus.we_assets} okText="Found — scene rendering ready" failText="Not found — use button below to download via steamcmd" />
        {!toolsStatus.we_assets && (
          <button
            onClick={handleDownloadAssets}
            disabled={downloadingAssets}
            style={{
              display: 'flex', alignItems: 'center', gap: 6,
              padding: '7px 12px',
              background: 'var(--bg-card)',
              border: '1px solid var(--border)',
              borderRadius: 5,
              color: 'var(--text-secondary)',
              fontSize: 12,
              cursor: downloadingAssets ? 'not-allowed' : 'pointer',
              alignSelf: 'flex-start',
            }}
          >
            <Download size={12} />
            Download WE Assets via steamcmd
          </button>
        )}
        <StatusRow label="steamcmd" ok={toolsStatus.steamcmd} okText="Installed — workshop downloads" failText="Not found — yay -S steamcmd" />
      </Section>

      <div style={{ display: 'flex', gap: 10, marginTop: 8 }}>
        <button
          onClick={handleSave}
          style={{
            padding: '9px 20px',
            background: 'var(--accent)',
            border: 'none',
            borderRadius: 5,
            color: '#fff',
            fontSize: 13,
            fontWeight: 600,
            cursor: 'pointer',
          }}
        >
          Save Settings
        </button>
        <button
          onClick={() => checkTools()}
          style={{
            padding: '9px 16px',
            background: 'transparent',
            border: '1px solid var(--border)',
            borderRadius: 5,
            color: 'var(--text-secondary)',
            fontSize: 13,
            cursor: 'pointer',
          }}
        >
          Refresh Tools
        </button>
      </div>
    </div>
  );
}

function Section({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <div style={{ marginBottom: 28 }}>
      <div style={{
        fontSize: 11,
        fontWeight: 600,
        color: 'var(--text-muted)',
        textTransform: 'uppercase',
        letterSpacing: '0.08em',
        marginBottom: 12,
        paddingBottom: 6,
        borderBottom: '1px solid var(--border)',
      }}>
        {title}
      </div>
      <div style={{ display: 'flex', flexDirection: 'column', gap: 14 }}>
        {children}
      </div>
    </div>
  );
}

function Field({ icon, label, hint, children }: {
  icon: React.ReactNode;
  label: string;
  hint?: React.ReactNode;
  children: React.ReactNode;
}) {
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 5 }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
        <span style={{ color: 'var(--text-muted)' }}>{icon}</span>
        <span style={{ fontSize: 12, fontWeight: 500, color: 'var(--text-primary)' }}>{label}</span>
      </div>
      {children}
      {hint && <div style={{ fontSize: 10, color: 'var(--text-muted)' }}>{hint}</div>}
    </div>
  );
}

function StatusRow({ label, ok, okText, failText }: {
  label: string; ok: boolean; okText: string; failText: string;
}) {
  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: 8, fontSize: 12 }}>
      <span style={{
        width: 6, height: 6, borderRadius: '50%',
        background: ok ? 'var(--success)' : '#666',
        flexShrink: 0,
      }} />
      <span style={{ color: 'var(--text-secondary)', minWidth: 100 }}>{label}</span>
      <span style={{ color: ok ? 'var(--text-muted)' : 'var(--warning)' }}>
        {ok ? okText : failText}
      </span>
    </div>
  );
}

function Toggle({ checked, onChange }: { checked: boolean; onChange: (v: boolean) => void }) {
  return (
    <div
      onClick={() => onChange(!checked)}
      style={{
        width: 36,
        height: 20,
        borderRadius: 10,
        background: checked ? 'var(--accent)' : 'var(--bg-card-hover)',
        border: '1px solid var(--border)',
        position: 'relative',
        cursor: 'pointer',
        transition: 'background 0.2s',
        flexShrink: 0,
      }}
    >
      <div style={{
        position: 'absolute',
        top: 2,
        left: checked ? 17 : 2,
        width: 14,
        height: 14,
        borderRadius: '50%',
        background: '#fff',
        transition: 'left 0.2s',
      }} />
    </div>
  );
}

const inputStyle: React.CSSProperties = {
  width: '100%',
  maxWidth: 400,
  padding: '6px 10px',
  background: 'var(--bg-card)',
  border: '1px solid var(--border)',
  borderRadius: 5,
  color: 'var(--text-primary)',
  fontSize: 12,
};
