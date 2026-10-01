import { useAppStore } from '../store/app-store';
import { CheckCircle, XCircle, Info } from 'lucide-react';

export default function Toast() {
  const toast = useAppStore((s) => s.toast);
  if (!toast) return null;

  const icons = {
    success: <CheckCircle size={14} color="var(--success)" />,
    error: <XCircle size={14} color="var(--error)" />,
    info: <Info size={14} color="var(--accent)" />,
  };

  const colors = {
    success: 'var(--success)',
    error: 'var(--error)',
    info: 'var(--accent)',
  };

  return (
    <div
      style={{
        position: 'fixed',
        bottom: 64,
        left: '50%',
        transform: 'translateX(-50%)',
        background: 'var(--bg-surface)',
        border: `1px solid ${colors[toast.type]}`,
        borderRadius: 8,
        padding: '10px 16px',
        display: 'flex',
        alignItems: 'center',
        gap: 8,
        color: 'var(--text-primary)',
        fontSize: 13,
        boxShadow: '0 4px 20px rgba(0,0,0,0.5)',
        zIndex: 9999,
        maxWidth: 400,
        animation: 'fadeIn 0.2s ease',
      }}
    >
      {icons[toast.type]}
      <span>{toast.message}</span>
      <style>{`@keyframes fadeIn { from { opacity: 0; transform: translateX(-50%) translateY(8px); } to { opacity: 1; transform: translateX(-50%) translateY(0); } }`}</style>
    </div>
  );
}
