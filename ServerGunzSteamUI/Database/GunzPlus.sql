CREATE TABLE [dbo].[CharacterPlus] (
    [CID] INT NOT NULL PRIMARY KEY,
    [PlusExpirationDate] DATETIME NOT NULL,
    [PlusColor] VARCHAR(7) DEFAULT '#FFFFFF', -- Mã Hex màu (#FF0000)
    [PurchaseCount] INT DEFAULT 0,            -- Đếm tổng số lần mua Plus
    [IsFirstPurchaseClaimed] BIT DEFAULT 0,   -- 1: Đã nhận quà lần đầu, 0: Chưa nhận
    [CreatedAt] DATETIME DEFAULT GETDATE(),
    [UpdatedAt] DATETIME DEFAULT GETDATE(),
    CONSTRAINT [FK_CharacterPlus_Character] FOREIGN KEY ([CID]) REFERENCES [dbo].[Character]([CID]) ON DELETE CASCADE
);