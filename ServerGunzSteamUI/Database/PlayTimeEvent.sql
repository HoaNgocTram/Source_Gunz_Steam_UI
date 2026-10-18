USE [GunzDB] -- Thay tên Database của anh vào đây nếu khác
GO

IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PlayTimeEvent]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PlayTimeEvent](
	[AID] [int] NOT NULL,                         -- Account ID của người chơi
	[AccumulatedMinutes] [int] NOT NULL DEFAULT 0, -- Số phút chơi tích lũy
	[NormalStateMask] [int] NOT NULL DEFAULT 0,    -- Bitmask trạng thái quà Thường (0: Chưa nhận, Bit=1: Đã nhận)
	[PlusStateMask] [int] NOT NULL DEFAULT 0,      -- Bitmask trạng thái quà GunZ Plus
	[LastUpdated] [datetime] NULL DEFAULT GETDATE(),-- Thời gian cập nhật gần nhất

    CONSTRAINT [PK_PlayTimeEvent] PRIMARY KEY CLUSTERED 
    (
	    [AID] ASC
    )
) ON [PRIMARY]
END
GO