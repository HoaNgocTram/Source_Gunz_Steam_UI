-- 1. Bảng lưu lịch sử điểm danh theo Nhân vật (CID)
CREATE TABLE [dbo].[CharacterAttendanceLog] (
    [CID] INT NOT NULL,
    [LogDate] DATE NOT NULL CONSTRAINT [DF_CharacterAttendanceLog_LogDate] DEFAULT (CAST(GETDATE() AS DATE)),
    CONSTRAINT [PK_CharacterAttendanceLog] PRIMARY KEY CLUSTERED ([CID] ASC, [LogDate] ASC)
);
GO

-- 2. SP Lấy thông tin điểm danh của Nhân vật
CREATE PROCEDURE [dbo].[spGetAttendanceInfoChar]
    @CID INT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @TotalDays INT = 0;
    SELECT @TotalDays = COUNT(DISTINCT LogDate) FROM CharacterAttendanceLog WHERE CID = @CID;

    DECLARE @IsTodayClaimed INT = 0;
    IF EXISTS (SELECT 1 FROM CharacterAttendanceLog WHERE CID = @CID AND LogDate = CAST(GETDATE() AS DATE))
    BEGIN
        SET @IsTodayClaimed = 1;
    END

    SELECT ISNULL(@TotalDays, 0) AS TotalDays, @IsTodayClaimed AS IsTodayClaimed;
END
GO

-- 3. SP Đánh dấu điểm danh ngày hôm nay cho Nhân vật
CREATE PROCEDURE [dbo].[spClaimAttendanceChar]
    @CID INT
AS
BEGIN
    SET NOCOUNT ON;

    IF NOT EXISTS (SELECT 1 FROM CharacterAttendanceLog WHERE CID = @CID AND LogDate = CAST(GETDATE() AS DATE))
    BEGIN
        INSERT INTO CharacterAttendanceLog (CID, LogDate) VALUES (@CID, CAST(GETDATE() AS DATE));
        SELECT 1 AS Result;
    END
    ELSE
    BEGIN
        SELECT 0 AS Result;
    END
END
GO